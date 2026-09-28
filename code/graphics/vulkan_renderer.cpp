#include "vulkan_renderer.hpp"

#include "utilities/externals.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <iostream>
#include <string>
#include <vector>

namespace {
bool g_glslangInitialized = false;

bool InitializeGlslang() {
  if (g_glslangInitialized) {
    return true;
  }

  g_glslangInitialized = glslang::InitializeProcess();

  if (!g_glslangInitialized) {
    std::cerr << "Failed to initialize glslang\n";
  }

  return g_glslangInitialized;
}

void ShutdownGlslang() {
  if (g_glslangInitialized) {
    glslang::FinalizeProcess();
    g_glslangInitialized = false;
  }
}

const char *VkResultName(VkResult result) {
  switch (result) {
  case VK_SUCCESS:
    return "VK_SUCCESS";
  case VK_NOT_READY:
    return "VK_NOT_READY";
  case VK_TIMEOUT:
    return "VK_TIMEOUT";
  case VK_EVENT_SET:
    return "VK_EVENT_SET";
  case VK_EVENT_RESET:
    return "VK_EVENT_RESET";
  case VK_INCOMPLETE:
    return "VK_INCOMPLETE";
  case VK_ERROR_OUT_OF_HOST_MEMORY:
    return "VK_ERROR_OUT_OF_HOST_MEMORY";
  case VK_ERROR_OUT_OF_DEVICE_MEMORY:
    return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
  case VK_ERROR_INITIALIZATION_FAILED:
    return "VK_ERROR_INITIALIZATION_FAILED";
  case VK_ERROR_DEVICE_LOST:
    return "VK_ERROR_DEVICE_LOST";
  case VK_ERROR_LAYER_NOT_PRESENT:
    return "VK_ERROR_LAYER_NOT_PRESENT";
  case VK_ERROR_EXTENSION_NOT_PRESENT:
    return "VK_ERROR_EXTENSION_NOT_PRESENT";
  case VK_ERROR_FEATURE_NOT_PRESENT:
    return "VK_ERROR_FEATURE_NOT_PRESENT";
  case VK_ERROR_INCOMPATIBLE_DRIVER:
    return "VK_ERROR_INCOMPATIBLE_DRIVER";
  case VK_ERROR_OUT_OF_DATE_KHR:
    return "VK_ERROR_OUT_OF_DATE_KHR";
  case VK_SUBOPTIMAL_KHR:
    return "VK_SUBOPTIMAL_KHR";
  case VK_ERROR_SURFACE_LOST_KHR:
    return "VK_ERROR_SURFACE_LOST_KHR";
  default:
    return "UNKNOWN_VK_RESULT";
  }
}
} // namespace

VulkanRenderer::VulkanRenderer(SDL_Window *window, bool enableValidation)
    : m_window(window), m_enableValidation(enableValidation) {}

VulkanRenderer::~VulkanRenderer() { Shutdown(); }

bool VulkanRenderer::InitInstance() {
  vkb::InstanceBuilder builder;
  builder.set_app_name("Uberrayet").require_api_version(1, 3, 0);
  if (m_enableValidation) {
    builder.request_validation_layers().use_default_debug_messenger();
  }
  auto instanceResult = builder.build();

  if (!instanceResult) {
    std::cerr << "Failed to create Vulkan instance: "
              << instanceResult.error().message() << '\n';
    return false;
  }

  m_vkbInstance = instanceResult.value();

  return true;
}

bool VulkanRenderer::InitSurface() {
  if (!SDL_Vulkan_CreateSurface(m_window, m_vkbInstance.instance, nullptr,
                                &m_surface)) {
    std::cerr << "SDL_Vulkan_CreateSurface failed: " << SDL_GetError() << '\n';
    return false;
  }

  return true;
}

bool VulkanRenderer::InitDevice() {
  VkPhysicalDeviceVulkan13Features features13{};
  features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
  features13.dynamicRendering = VK_TRUE;
  features13.synchronization2 = VK_TRUE;

  vkb::PhysicalDeviceSelector selector{m_vkbInstance};

  auto physicalDeviceResult = selector.set_surface(m_surface)
                                  .set_minimum_version(1, 3)
                                  .set_required_features_13(features13)
                                  .select();

  if (!physicalDeviceResult) {
    std::cerr << "Failed to select physical device: "
              << physicalDeviceResult.error().message() << '\n';
    return false;
  }

  vkb::DeviceBuilder deviceBuilder{physicalDeviceResult.value()};

  auto deviceResult = deviceBuilder.build();

  if (!deviceResult) {
    std::cerr << "Failed to create logical device: "
              << deviceResult.error().message() << '\n';
    return false;
  }

  m_vkbDevice = deviceResult.value();

  auto queueResult = m_vkbDevice.get_queue(vkb::QueueType::graphics);

  if (!queueResult) {
    std::cerr << "Failed to get graphics queue: "
              << queueResult.error().message() << '\n';
    return false;
  }

  m_graphicsQueue = queueResult.value();

  auto queueFamilyResult =
      m_vkbDevice.get_queue_index(vkb::QueueType::graphics);

  if (!queueFamilyResult) {
    std::cerr << "Failed to get graphics queue family: "
              << queueFamilyResult.error().message() << '\n';
    return false;
  }

  m_graphicsQueueFamily = queueFamilyResult.value();

  return true;
}

bool VulkanRenderer::InitSwapchain() {
  int width = 0;
  int height = 0;

  SDL_GetWindowSizeInPixels(m_window, &width, &height);

  vkb::SwapchainBuilder swapchainBuilder{m_vkbDevice};

  auto swapchainResult =
      swapchainBuilder
          .set_desired_format(
              {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
          .set_desired_extent(width, height)
          .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
          .build();

  if (!swapchainResult) {
    std::cerr << "Failed to create swapchain: "
              << swapchainResult.error().message() << '\n';
    return false;
  }

  m_vkbSwapchain = swapchainResult.value();

  auto imagesResult = m_vkbSwapchain.get_images();

  if (!imagesResult) {
    std::cerr << "Failed to retrieve swapchain images: "
              << imagesResult.error().message() << '\n';
    return false;
  }

  m_swapchainImages = imagesResult.value();

  auto imageViewsResult = m_vkbSwapchain.get_image_views();

  if (!imageViewsResult) {
    std::cerr << "Failed to retrieve swapchain image views: "
              << imageViewsResult.error().message() << '\n';
    return false;
  }

  m_swapchainImageViews = imageViewsResult.value();

  return true;
}

bool VulkanRenderer::CreateAllocator() {
  VmaAllocatorCreateInfo allocatorInfo{};
  allocatorInfo.instance = m_vkbInstance.instance;
  allocatorInfo.physicalDevice = m_vkbDevice.physical_device;
  allocatorInfo.device = m_vkbDevice.device;
  allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;

  const VkResult result = vmaCreateAllocator(&allocatorInfo, &m_allocator);

  if (result != VK_SUCCESS) {
    std::cerr << "vmaCreateAllocator failed: " << result << '\n';
    m_allocator = VK_NULL_HANDLE;
    return false;
  }

  return true;
}

bool VulkanRenderer::CreateMeshBuffers(const std::vector<Vertex> &vertices,
                                       const std::vector<unsigned int> &indices,
                                       VulkanMeshResource &resource) {
  resource = {};

  if (m_allocator == VK_NULL_HANDLE) {
    std::cerr << "Cannot upload mesh: VMA allocator is null\n";
    return false;
  }

  if (vertices.empty()) {
    std::cerr << "Cannot upload mesh: vertex list is empty\n";
    return false;
  }

  if (indices.empty()) {
    std::cerr << "Cannot upload mesh: index list is empty\n";
    return false;
  }

  const VkDeviceSize vertexBufferSize =
      static_cast<VkDeviceSize>(sizeof(Vertex) * vertices.size());

  VkBufferCreateInfo vertexBufferInfo{};
  vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  vertexBufferInfo.pNext = nullptr;
  vertexBufferInfo.flags = 0;
  vertexBufferInfo.size = vertexBufferSize;
  vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  vertexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo vertexAllocationInfo{};
  vertexAllocationInfo.flags =
      VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
      VMA_ALLOCATION_CREATE_MAPPED_BIT;
  vertexAllocationInfo.usage = VMA_MEMORY_USAGE_AUTO;

  VmaAllocationInfo vertexMemoryInfo{};

  VkResult result = vmaCreateBuffer(
      m_allocator, &vertexBufferInfo, &vertexAllocationInfo,
      &resource.vertexBuffer, &resource.vertexAllocation, &vertexMemoryInfo);

  if (result != VK_SUCCESS) {
    std::cerr << "Failed to create vertex buffer: " << VkResultName(result)
              << " (" << result << ")\n";
    resource = {};
    return false;
  }

  if (vertexMemoryInfo.pMappedData == nullptr) {
    std::cerr << "Vertex allocation was not mapped\n";

    vmaDestroyBuffer(m_allocator, resource.vertexBuffer,
                     resource.vertexAllocation);

    resource = {};
    return false;
  }

  std::memcpy(vertexMemoryInfo.pMappedData, vertices.data(),
              static_cast<std::size_t>(vertexBufferSize));

  vmaFlushAllocation(m_allocator, resource.vertexAllocation, 0, VK_WHOLE_SIZE);

  const VkDeviceSize indexBufferSize =
      static_cast<VkDeviceSize>(sizeof(unsigned int) * indices.size());

  VkBufferCreateInfo indexBufferInfo{};
  indexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  indexBufferInfo.pNext = nullptr;
  indexBufferInfo.flags = 0;
  indexBufferInfo.size = indexBufferSize;
  indexBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  indexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo indexAllocationInfo{};
  indexAllocationInfo.flags =
      VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
      VMA_ALLOCATION_CREATE_MAPPED_BIT;
  indexAllocationInfo.usage = VMA_MEMORY_USAGE_AUTO;

  VmaAllocationInfo indexMemoryInfo{};

  result = vmaCreateBuffer(m_allocator, &indexBufferInfo, &indexAllocationInfo,
                           &resource.indexBuffer, &resource.indexAllocation,
                           &indexMemoryInfo);

  if (result != VK_SUCCESS) {
    std::cerr << "Failed to create index buffer: " << VkResultName(result)
              << " (" << result << ")\n";

    vmaDestroyBuffer(m_allocator, resource.vertexBuffer,
                     resource.vertexAllocation);

    resource = {};
    return false;
  }

  if (indexMemoryInfo.pMappedData == nullptr) {
    std::cerr << "Index allocation was not mapped\n";

    vmaDestroyBuffer(m_allocator, resource.vertexBuffer,
                     resource.vertexAllocation);

    vmaDestroyBuffer(m_allocator, resource.indexBuffer,
                     resource.indexAllocation);

    resource = {};
    return false;
  }

  std::memcpy(indexMemoryInfo.pMappedData, indices.data(),
              static_cast<std::size_t>(indexBufferSize));

  vmaFlushAllocation(m_allocator, resource.vertexAllocation, 0, VK_WHOLE_SIZE);

  resource.indexCount = static_cast<std::uint32_t>(indices.size());

  return true;
}

bool VulkanRenderer::CreateFrameResources() {
  VkCommandPoolCreateInfo commandPoolInfo{};
  commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  commandPoolInfo.queueFamilyIndex = m_graphicsQueueFamily;

  VkResult result = vkCreateCommandPool(m_vkbDevice.device, &commandPoolInfo,
                                        nullptr, &m_commandPool);

  if (result != VK_SUCCESS) {
    std::cerr << "vkCreateCommandPool failed: " << VkResultName(result) << " ("
              << result << ")\n";
    return false;
  }

  VkCommandBufferAllocateInfo commandBufferInfo{};
  commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  commandBufferInfo.commandPool = m_commandPool;
  commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  commandBufferInfo.commandBufferCount = 1;

  result = vkAllocateCommandBuffers(m_vkbDevice.device, &commandBufferInfo,
                                    &m_commandBuffer);

  if (result != VK_SUCCESS) {
    std::cerr << "vkAllocateCommandBuffers failed: " << VkResultName(result)
              << " (" << result << ")\n";
    return false;
  }

  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  result = vkCreateSemaphore(m_vkbDevice.device, &semaphoreInfo, nullptr,
                             &m_imageAvailableSemaphore);

  if (result != VK_SUCCESS) {
    std::cerr << "vkCreateSemaphore(image available) failed: "
              << VkResultName(result) << " (" << result << ")\n";
    return false;
  }

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  result =
      vkCreateFence(m_vkbDevice.device, &fenceInfo, nullptr, &m_inFlightFence);

  if (result != VK_SUCCESS) {
    std::cerr << "vkCreateFence failed: " << VkResultName(result) << " ("
              << result << ")\n";
    return false;
  }

  if (!CreateRenderFinishedSemaphores()) {
    return false;
  }

  return true;
}

void VulkanRenderer::DestroyMeshResource(VulkanMeshResource &resource) {
  if (m_allocator == VK_NULL_HANDLE) {
    return;
  }

  if (resource.vertexBuffer != VK_NULL_HANDLE) {
    vmaDestroyBuffer(m_allocator, resource.vertexBuffer,
                     resource.vertexAllocation);

    resource.vertexBuffer = VK_NULL_HANDLE;
    resource.vertexAllocation = VK_NULL_HANDLE;
  }

  if (resource.indexBuffer != VK_NULL_HANDLE) {
    vmaDestroyBuffer(m_allocator, resource.indexBuffer,
                     resource.indexAllocation);

    resource.indexBuffer = VK_NULL_HANDLE;
    resource.indexAllocation = VK_NULL_HANDLE;
  }

  resource.indexCount = 0;
}

MeshHandle
VulkanRenderer::UploadMesh(const std::vector<Vertex> &vertices,
                           const std::vector<unsigned int> &indices) {
  VulkanMeshResource resource{};
  if (!CreateMeshBuffers(vertices, indices, resource)) {
    return kInvalidMesh;
  }
  m_meshResources.push_back(resource);
  return static_cast<MeshHandle>(
      m_meshResources.size()); // 1-based; 0 stays "invalid"
}

void VulkanRenderer::DrawMesh(MeshHandle meshHandle, TextureHandle) {
  if (!m_renderingActive) {
    return;
  }

  if (meshHandle == kInvalidMesh) {
    return;
  }

  const std::size_t resourceIndex = static_cast<std::size_t>(meshHandle) - 1;

  if (resourceIndex >= m_meshResources.size()) {
    std::cerr << "Invalid mesh handle: " << meshHandle << '\n';
    return;
  }

  const VulkanMeshResource &resource = m_meshResources[resourceIndex];

  if (resource.vertexBuffer == VK_NULL_HANDLE ||
      resource.indexBuffer == VK_NULL_HANDLE || resource.indexCount == 0) {
    return;
  }

  const VkDeviceSize offset = 0;

  vkCmdBindVertexBuffers(m_commandBuffer, 0, 1, &resource.vertexBuffer,
                         &offset);

  vkCmdBindIndexBuffer(m_commandBuffer, resource.indexBuffer, 0,
                       VK_INDEX_TYPE_UINT32);

  vkCmdDrawIndexed(m_commandBuffer, resource.indexCount, 1, 0, 0, 0);
}

bool VulkanRenderer::CreateRenderFinishedSemaphores() {
  for (VkSemaphore semaphore : m_renderFinishedSemaphores) {
    if (semaphore != VK_NULL_HANDLE) {
      vkDestroySemaphore(m_vkbDevice.device, semaphore, nullptr);
    }
  }

  m_renderFinishedSemaphores.clear();
  m_renderFinishedSemaphores.resize(m_swapchainImages.size(), VK_NULL_HANDLE);

  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  for (VkSemaphore &semaphore : m_renderFinishedSemaphores) {
    const VkResult result = vkCreateSemaphore(
        m_vkbDevice.device, &semaphoreInfo, nullptr, &semaphore);

    if (result != VK_SUCCESS) {
      std::cerr << "vkCreateSemaphore(render finished) failed: "
                << VkResultName(result) << " (" << result << ")\n";

      for (VkSemaphore created : m_renderFinishedSemaphores) {
        if (created != VK_NULL_HANDLE) {
          vkDestroySemaphore(m_vkbDevice.device, created, nullptr);
        }
      }

      m_renderFinishedSemaphores.clear();
      return false;
    }
  }

  return true;
}

bool VulkanRenderer::RecreateSwapchain() {
  int width = 0, height = 0;
  SDL_GetWindowSizeInPixels(m_window, &width, &height);
  if (width == 0 || height == 0) {
    return false; // minimised; try again next frame
  }

  vkDeviceWaitIdle(m_vkbDevice.device);

  vkb::Swapchain oldSwapchain = m_vkbSwapchain;
  std::vector<VkImageView> oldViews = std::move(m_swapchainImageViews);

  vkb::SwapchainBuilder builder{m_vkbDevice};
  auto result = builder.set_old_swapchain(oldSwapchain)
                    .set_desired_format({VK_FORMAT_B8G8R8A8_UNORM,
                                         VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
                    .set_desired_extent(width, height)
                    .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
                    .build();

  if (!result) {
    std::cerr << "Swapchain recreation failed: " << result.error().message()
              << '\n';
    m_swapchainImageViews = std::move(oldViews); // keep the old one alive
    return false;
  }

  oldSwapchain.destroy_image_views(oldViews);
  vkb::destroy_swapchain(oldSwapchain);

  m_vkbSwapchain = result.value();
  m_swapchainImages = m_vkbSwapchain.get_images().value();
  m_swapchainImageViews = m_vkbSwapchain.get_image_views().value();
  m_swapchainDirty = false;

  return CreateRenderFinishedSemaphores(); // image count may have changed
}

void VulkanRenderer::OnResize(int width, int height) {
  if (static_cast<std::uint32_t>(width) != m_vkbSwapchain.extent.width ||
      static_cast<std::uint32_t>(height) != m_vkbSwapchain.extent.height) {
    m_swapchainDirty = true; // rebuilt lazily at the start of the next frame
  }
}

void VulkanRenderer::DestroyMesh(MeshHandle meshHandle) {
  if (meshHandle == kInvalidMesh) {
    return;
  }

  const std::size_t resourceIndex = static_cast<std::size_t>(meshHandle) - 1;

  if (resourceIndex >= m_meshResources.size()) {
    return;
  }

  if (m_vkbDevice.device != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(m_vkbDevice.device);
  }

  DestroyMeshResource(m_meshResources[resourceIndex]);
}

bool VulkanRenderer::Initialize() {
  if (!InitializeGlslang()) {
    return false;
  }

  if (!InitInstance()) {
    std::cerr << "Vulkan instance initialization failed\n";
    return false;
  }

  if (!InitSurface()) {
    std::cerr << "Vulkan surface initialization failed\n";
    return false;
  }

  if (!InitDevice()) {
    std::cerr << "Vulkan device initialization failed\n";
    return false;
  }

  if (!CreateAllocator()) {
    std::cerr << "VMA allocator initialization failed\n";
    return false;
  }

  if (!InitSwapchain()) {
    std::cerr << "Vulkan swapchain initialization failed\n";
    return false;
  }

  if (!CreateFrameResources()) {
    std::cerr << "Vulkan frame-resource initialization failed\n";
    return false;
  }

  std::cout << "Vulkan initialized: " << m_vkbDevice.physical_device.name
            << '\n';

  return true;
}

bool VulkanRenderer::CompileShader(const std::string &source, EShLanguage stage,
                                   const std::string &filename,
                                   std::vector<std::uint32_t> &output) {
  if (source.empty()) {
    std::cerr << "Shader source is empty: " << filename << '\n';
    return false;
  }

  glslang::TShader shader(stage);

  const char *sourceString = source.c_str();

  shader.setStrings(&sourceString, 1);

  shader.setSourceFile(filename.c_str());

  shader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientVulkan,
                     100);

  shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);

  shader.setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_6);

  const EShMessages messages =
      static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);

  const TBuiltInResource *resources = GetDefaultResources();

  if (!shader.parse(resources, 460, false, messages)) {
    std::cerr << "Shader parse error in " << filename << ":\n"
              << shader.getInfoLog() << shader.getInfoDebugLog() << '\n';
    return false;
  }

  glslang::TProgram program;
  program.addShader(&shader);

  if (!program.link(messages)) {
    std::cerr << "Shader link error in " << filename << ":\n"
              << program.getInfoLog() << program.getInfoDebugLog() << '\n';
    return false;
  }

  const glslang::TIntermediate *intermediate = program.getIntermediate(stage);

  if (intermediate == nullptr) {
    std::cerr << "No shader intermediate generated for " << filename << '\n';
    return false;
  }

  output.clear();

  glslang::SpvOptions options{};
  options.generateDebugInfo = true;
  options.disableOptimizer = false;
  options.optimizeSize = false;

  glslang::GlslangToSpv(*intermediate, output, &options);

  if (output.empty()) {
    std::cerr << "Generated empty SPIR-V for " << filename << '\n';
    return false;
  }

  return true;
}

bool VulkanRenderer::CreateShaderModule(const std::vector<std::uint32_t> &code,
                                        VkShaderModule &module) {
  module = VK_NULL_HANDLE;

  if (code.empty()) {
    std::cerr << "Cannot create shader module from empty SPIR-V\n";
    return false;
  }

  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size() * sizeof(std::uint32_t);
  createInfo.pCode = code.data();

  const VkResult result =
      vkCreateShaderModule(m_vkbDevice.device, &createInfo, nullptr, &module);

  if (result != VK_SUCCESS) {
    std::cerr << "vkCreateShaderModule failed: " << VkResultName(result) << " ("
              << result << ")\n";
    module = VK_NULL_HANDLE;
    return false;
  }

  return true;
}

bool VulkanRenderer::CreatePipelineLayout() {
  if (m_pipelineLayout != VK_NULL_HANDLE) {
    return true;
  }

  VkPipelineLayoutCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

  const VkResult result = vkCreatePipelineLayout(
      m_vkbDevice.device, &createInfo, nullptr, &m_pipelineLayout);

  if (result != VK_SUCCESS) {
    std::cerr << "vkCreatePipelineLayout failed: " << VkResultName(result)
              << " (" << result << ")\n";
    m_pipelineLayout = VK_NULL_HANDLE;
    return false;
  }

  return true;
}

bool VulkanRenderer::CreateShaderProgram(const std::string &vertexPath,
                                         const std::string &fragmentPath) {
  const std::string vertexSource = LoadShaderSource(vertexPath);

  const std::string fragmentSource = LoadShaderSource(fragmentPath);

  if (vertexSource.empty() || fragmentSource.empty()) {
    return false;
  }

  std::vector<std::uint32_t> vertexSpirv;
  std::vector<std::uint32_t> fragmentSpirv;

  if (!CompileShader(vertexSource, EShLangVertex, vertexPath, vertexSpirv)) {
    return false;
  }

  if (!CompileShader(fragmentSource, EShLangFragment, fragmentPath,
                     fragmentSpirv)) {
    return false;
  }

  VkShaderModule vertexModule = VK_NULL_HANDLE;

  VkShaderModule fragmentModule = VK_NULL_HANDLE;

  if (!CreateShaderModule(vertexSpirv, vertexModule)) {
    return false;
  }

  if (!CreateShaderModule(fragmentSpirv, fragmentModule)) {
    vkDestroyShaderModule(m_vkbDevice.device, vertexModule, nullptr);
    return false;
  }

  if (!CreatePipelineLayout()) {
    vkDestroyShaderModule(m_vkbDevice.device, vertexModule, nullptr);

    vkDestroyShaderModule(m_vkbDevice.device, fragmentModule, nullptr);

    return false;
  }

  VkPipelineShaderStageCreateInfo shaderStages[2]{};

  shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  shaderStages[0].module = vertexModule;
  shaderStages[0].pName = "main";

  shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  shaderStages[1].module = fragmentModule;
  shaderStages[1].pName = "main";

  VkVertexInputBindingDescription binding{};
  binding.binding = 0;
  binding.stride = sizeof(Vertex);
  binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  VkVertexInputAttributeDescription attributes[3]{};

  attributes[0].location = 0;
  attributes[0].binding = 0;
  attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributes[0].offset = offsetof(Vertex, x);

  attributes[1].location = 1;
  attributes[1].binding = 0;
  attributes[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
  attributes[1].offset = offsetof(Vertex, red);

  attributes[2].location = 2;
  attributes[2].binding = 0;
  attributes[2].format = VK_FORMAT_R32G32_SFLOAT;
  attributes[2].offset = offsetof(Vertex, u);

  VkPipelineVertexInputStateCreateInfo vertexInput{};
  vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInput.vertexBindingDescriptionCount = 1;
  vertexInput.pVertexBindingDescriptions = &binding;
  vertexInput.vertexAttributeDescriptionCount = 3;
  vertexInput.pVertexAttributeDescriptions = attributes;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;

  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.cullMode = VK_CULL_MODE_NONE;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.lineWidth = 1.0f;

  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.sampleShadingEnable = VK_FALSE;

  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.blendEnable = VK_FALSE;
  colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  VkPipelineColorBlendStateCreateInfo colorBlend{};
  colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlend.logicOpEnable = VK_FALSE;
  colorBlend.logicOp = VK_LOGIC_OP_COPY;
  colorBlend.attachmentCount = 1;
  colorBlend.pAttachments = &colorBlendAttachment;

  const VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                          VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = 2;
  dynamicState.pDynamicStates = dynamicStates;

  const VkFormat colorFormat = m_vkbSwapchain.image_format;

  VkPipelineRenderingCreateInfo renderingInfo{};
  renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
  renderingInfo.colorAttachmentCount = 1;
  renderingInfo.pColorAttachmentFormats = &colorFormat;

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.pNext = &renderingInfo;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInput;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pColorBlendState = &colorBlend;
  pipelineInfo.pDynamicState = &dynamicState;
  pipelineInfo.layout = m_pipelineLayout;
  pipelineInfo.renderPass = VK_NULL_HANDLE;
  pipelineInfo.subpass = 0;

  VkPipeline newPipeline = VK_NULL_HANDLE;

  const VkResult result =
      vkCreateGraphicsPipelines(m_vkbDevice.device, VK_NULL_HANDLE, 1,
                                &pipelineInfo, nullptr, &newPipeline);

  vkDestroyShaderModule(m_vkbDevice.device, vertexModule, nullptr);

  vkDestroyShaderModule(m_vkbDevice.device, fragmentModule, nullptr);

  if (result != VK_SUCCESS) {
    std::cerr << "vkCreateGraphicsPipelines failed: " << VkResultName(result)
              << " (" << result << ")\n";

    return false;
  }

  if (m_pipeline != VK_NULL_HANDLE) {
    vkDestroyPipeline(m_vkbDevice.device, m_pipeline, nullptr);
  }

  m_pipeline = newPipeline;

  std::cout << "Graphics pipeline created successfully\n";

  return true;
}

void VulkanRenderer::BeginFrame() {
  m_frameActive = false;
  m_renderingActive = false;

  if (m_swapchainDirty && !RecreateSwapchain()) {
    return;
  }

  vkWaitForFences(m_vkbDevice.device, 1, &m_inFlightFence, VK_TRUE, UINT64_MAX);

  const VkResult result = vkAcquireNextImageKHR(
      m_vkbDevice.device, m_vkbSwapchain.swapchain, UINT64_MAX,
      m_imageAvailableSemaphore, VK_NULL_HANDLE, &m_currentImageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    m_swapchainDirty = true;
    return; // fence untouched, so the next wait can't deadlock
  }
  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    std::cerr << "vkAcquireNextImageKHR failed: " << VkResultName(result)
              << '\n';
    return;
  }
  if (result == VK_SUBOPTIMAL_KHR) {
    m_swapchainDirty = true; // render this frame, rebuild before the next
  }

  vkResetFences(m_vkbDevice.device, 1,
                &m_inFlightFence); // only now: we will submit

  vkResetCommandBuffer(m_commandBuffer, 0);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vkBeginCommandBuffer(m_commandBuffer, &beginInfo);

  m_frameActive = true;
}

void VulkanRenderer::Clear(float red, float green, float blue, float alpha) {
  if (!m_frameActive) {
    return;
  }

  VkImageMemoryBarrier2 barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
  barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
  barrier.srcAccessMask = VK_ACCESS_2_NONE;
  barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
  barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
  barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  barrier.image = m_swapchainImages[m_currentImageIndex];
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.layerCount = 1;
  barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

  VkDependencyInfo dependency{};
  dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
  dependency.imageMemoryBarrierCount = 1;
  dependency.pImageMemoryBarriers = &barrier;

  vkCmdPipelineBarrier2(m_commandBuffer, &dependency);

  VkClearValue clearValue{};
  clearValue.color.float32[0] = red;
  clearValue.color.float32[1] = green;
  clearValue.color.float32[2] = blue;
  clearValue.color.float32[3] = alpha;

  VkRenderingAttachmentInfo colorAttachment{};
  colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  colorAttachment.imageView = m_swapchainImageViews[m_currentImageIndex];
  colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.clearValue = clearValue;

  int width = 0;
  int height = 0;

  SDL_GetWindowSizeInPixels(m_window, &width, &height);

  VkRenderingInfo rendering{};
  rendering.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
  rendering.renderArea.extent.width = static_cast<std::uint32_t>(width);
  rendering.renderArea.extent.height = static_cast<std::uint32_t>(height);
  rendering.layerCount = 1;
  rendering.colorAttachmentCount = 1;
  rendering.pColorAttachments = &colorAttachment;
  rendering.renderArea.extent = m_vkbSwapchain.extent;


  vkCmdBeginRendering(m_commandBuffer, &rendering);
  m_renderingActive = true;
}

void VulkanRenderer::UseShaderProgram() {
  if (!m_renderingActive || m_pipeline == VK_NULL_HANDLE) {
    return;
  }

  const VkExtent2D extent = m_vkbSwapchain.extent;

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = static_cast<float>(extent.height);    // negative height flips Y,
  viewport.width = static_cast<float>(extent.width); // so +Y is up like OpenGL
  viewport.height = -static_cast<float>(extent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.extent = extent;

  vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
  vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    m_pipeline);
}

void VulkanRenderer::EndFrame() {
  if (!m_frameActive) {
    return;
  }

  if (m_renderingActive) {
    vkCmdEndRendering(m_commandBuffer);
    m_renderingActive = false;
  }

  VkImageMemoryBarrier2 barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
  barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
  barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
  barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
  barrier.dstAccessMask = VK_ACCESS_2_NONE;
  barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  barrier.image = m_swapchainImages[m_currentImageIndex];
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  VkDependencyInfo dependency{};
  dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
  dependency.imageMemoryBarrierCount = 1;
  dependency.pImageMemoryBarriers = &barrier;

  vkCmdPipelineBarrier2(m_commandBuffer, &dependency);

  VkResult result = vkEndCommandBuffer(m_commandBuffer);

  if (result != VK_SUCCESS) {
    std::cerr << "vkEndCommandBuffer failed: " << VkResultName(result) << " ("
              << result << ")\n";
    m_frameActive = false;
    return;
  }

  if (m_currentImageIndex >= m_renderFinishedSemaphores.size()) {
    std::cerr << "Invalid swapchain image index: " << m_currentImageIndex
              << '\n';
    m_frameActive = false;
    return;
  }

  const VkSemaphore renderFinishedSemaphore =
      m_renderFinishedSemaphores[m_currentImageIndex];

  VkPipelineStageFlags waitStage =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  VkSubmitInfo submit{};
  submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit.waitSemaphoreCount = 1;
  submit.pWaitSemaphores = &m_imageAvailableSemaphore;
  submit.pWaitDstStageMask = &waitStage;
  submit.commandBufferCount = 1;
  submit.pCommandBuffers = &m_commandBuffer;
  submit.signalSemaphoreCount = 1;
  submit.pSignalSemaphores = &renderFinishedSemaphore;

  result = vkQueueSubmit(m_graphicsQueue, 1, &submit, m_inFlightFence);

  if (result != VK_SUCCESS) {
    std::cerr << "vkQueueSubmit failed: " << VkResultName(result) << " ("
              << result << ")\n";
    m_frameActive = false;
    return;
  }

  VkPresentInfoKHR present{};
  present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present.waitSemaphoreCount = 1;
  present.pWaitSemaphores = &renderFinishedSemaphore;
  present.swapchainCount = 1;
  present.pSwapchains = &m_vkbSwapchain.swapchain;
  present.pImageIndices = &m_currentImageIndex;

  const VkResult presentResult = vkQueuePresentKHR(m_graphicsQueue, &present);

  if (presentResult == VK_ERROR_OUT_OF_DATE_KHR ||
      presentResult == VK_SUBOPTIMAL_KHR ||
      presentResult == VK_ERROR_SURFACE_LOST_KHR) {
    m_swapchainDirty = true;
  } else if (presentResult != VK_SUCCESS) {
    std::cerr << "vkQueuePresentKHR failed: " << VkResultName(presentResult)
              << " (" << presentResult << ")\n";
  }

  m_frameActive = false;
}

TextureHandle VulkanRenderer::UploadTexture(const unsigned char *, int, int) {
  return kInvalidTexture;
}

void VulkanRenderer::DestroyTexture(TextureHandle) {}

void VulkanRenderer::Shutdown() {
  if (m_vkbDevice.device != VK_NULL_HANDLE) {
    const VkResult waitResult = vkDeviceWaitIdle(m_vkbDevice.device);

    if (waitResult != VK_SUCCESS) {
      std::cerr << "vkDeviceWaitIdle failed during shutdown: "
                << VkResultName(waitResult) << " (" << waitResult << ")\n";
    }

    for (VulkanMeshResource &resource : m_meshResources) {
      DestroyMeshResource(resource);
    }

    m_meshResources.clear();

    if (m_pipeline != VK_NULL_HANDLE) {
      vkDestroyPipeline(m_vkbDevice.device, m_pipeline, nullptr);
      m_pipeline = VK_NULL_HANDLE;
    }

    if (m_pipelineLayout != VK_NULL_HANDLE) {
      vkDestroyPipelineLayout(m_vkbDevice.device, m_pipelineLayout, nullptr);
      m_pipelineLayout = VK_NULL_HANDLE;
    }

    if (m_inFlightFence != VK_NULL_HANDLE) {
      vkDestroyFence(m_vkbDevice.device, m_inFlightFence, nullptr);
      m_inFlightFence = VK_NULL_HANDLE;
    }

    if (m_imageAvailableSemaphore != VK_NULL_HANDLE) {
      vkDestroySemaphore(m_vkbDevice.device, m_imageAvailableSemaphore,
                         nullptr);
      m_imageAvailableSemaphore = VK_NULL_HANDLE;
    }

    for (VkSemaphore semaphore : m_renderFinishedSemaphores) {
      if (semaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(m_vkbDevice.device, semaphore, nullptr);
      }
    }

    m_renderFinishedSemaphores.clear();

    if (m_commandPool != VK_NULL_HANDLE) {
      vkDestroyCommandPool(m_vkbDevice.device, m_commandPool, nullptr);
      m_commandPool = VK_NULL_HANDLE;
      m_commandBuffer = VK_NULL_HANDLE;
    }

    if (m_allocator != VK_NULL_HANDLE) {
      vmaDestroyAllocator(m_allocator);
      m_allocator = VK_NULL_HANDLE;
    }

    if (m_vkbSwapchain.swapchain != VK_NULL_HANDLE) {
      m_vkbSwapchain.destroy_image_views(m_swapchainImageViews);

      m_swapchainImageViews.clear();
      m_swapchainImages.clear();

      vkb::destroy_swapchain(m_vkbSwapchain);
      m_vkbSwapchain = {};
    }

    vkb::destroy_device(m_vkbDevice);
    m_vkbDevice = {};
  }

  if (m_surface != VK_NULL_HANDLE) {
    vkb::destroy_surface(m_vkbInstance, m_surface);
    m_surface = VK_NULL_HANDLE;
  }

  if (m_vkbInstance.instance != VK_NULL_HANDLE) {
    vkb::destroy_instance(m_vkbInstance);
    m_vkbInstance = {};
  }

  ShutdownGlslang();
}