#pragma once

#include "irenderer.hpp"

#include <SDL3/SDL.h>
#include <VkBootstrap.h>
#include <vulkan/vulkan.h>

#include <glslang/Public/ShaderLang.h>

#include <cstdint>
#include <string>
#include <vector>

#include <vk_mem_alloc.h>

struct VulkanMeshResource {
  VkBuffer vertexBuffer = VK_NULL_HANDLE;
  VmaAllocation vertexAllocation = VK_NULL_HANDLE;

  VkBuffer indexBuffer = VK_NULL_HANDLE;
  VmaAllocation indexAllocation = VK_NULL_HANDLE;

  std::uint32_t indexCount = 0;
};

struct VulkanTextureResource {
  VkImage image = VK_NULL_HANDLE;
  VmaAllocation allocation = VK_NULL_HANDLE;
  VkImageView view = VK_NULL_HANDLE;
  VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
};

class VulkanRenderer final : public IRenderer {
public:
  VulkanRenderer(SDL_Window *window, bool enableValidation);

  ~VulkanRenderer() override;

  bool Initialize() override;

  bool CreateShaderProgram(const std::string &vertexPath,
                           const std::string &fragmentPath) override;

  void BeginFrame() override;

  void Clear(float red, float green, float blue, float alpha) override;

  void UseShaderProgram() override;
  void EndFrame() override;
  void OnResize(int width, int height) override;
  void Shutdown() override;

  MeshHandle UploadMesh(const std::vector<Vertex> &vertices,
                        const std::vector<unsigned int> &indices) override;

  void DestroyMesh(MeshHandle handle) override;

  TextureHandle UploadTexture(const unsigned char *pixels, int width,
                              int height) override;

  void DestroyTexture(TextureHandle handle) override;

  void DrawMesh(MeshHandle mesh, TextureHandle texture,
                const mat4 &mvp) override;

private:
  bool InitInstance();
  bool InitSurface();
  bool InitDevice();
  bool InitSwapchain();

  bool CreateAllocator();
  bool CreateFrameResources();

  bool CreateMeshBuffers(const std::vector<Vertex> &vertices,
                         const std::vector<unsigned int> &indices,
                         VulkanMeshResource &resource);

  void DestroyMeshResource(VulkanMeshResource &resource);

  bool CompileShader(const std::string &source, EShLanguage stage,
                     const std::string &filename,
                     std::vector<std::uint32_t> &output);

  bool CreateShaderModule(const std::vector<std::uint32_t> &code,
                          VkShaderModule &module);
                          
  bool CreatePipelineLayout();

  bool CreateDescriptorSetLayout();
  bool CreateSampler();
  bool CreateDescriptorPool();
  bool AllocateDescriptorSet(VulkanTextureResource &resource,
                             VkSampler samplerToUse);
  bool CreateFallbackTexture();

  TextureHandle UploadTextureInternal(const unsigned char *pixels, int width,
                                      int height, bool useNearestFilter);

  void DestroyTextureResource(VulkanTextureResource &resource);

  bool CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width,
                         uint32_t height);
  bool TransitionImageLayout(VkImage image, VkImageLayout oldLayout,
                             VkImageLayout newLayout);
  VkCommandBuffer BeginOneTimeCommands();
  void EndOneTimeCommands(VkCommandBuffer cmd);
  
  bool RecreateSwapchain();
  bool CreateRenderFinishedSemaphores();
private:
  // members
  bool m_swapchainDirty = false;
  bool m_renderingActive = false;
  SDL_Window *m_window = nullptr;
  bool m_enableValidation = false;

  vkb::Instance m_vkbInstance{};
  vkb::Device m_vkbDevice{};
  vkb::Swapchain m_vkbSwapchain{};

  VkSurfaceKHR m_surface = VK_NULL_HANDLE;
  VkQueue m_graphicsQueue = VK_NULL_HANDLE;
  std::uint32_t m_graphicsQueueFamily = 0;

  std::vector<VkImage> m_swapchainImages;
  std::vector<VkImageView> m_swapchainImageViews;

  VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

  VkPipeline m_pipeline = VK_NULL_HANDLE;

  VmaAllocator m_allocator = VK_NULL_HANDLE;

  std::vector<VulkanMeshResource> m_meshResources;

  VkCommandPool m_commandPool = VK_NULL_HANDLE;
  VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;

  VkSemaphore m_imageAvailableSemaphore = VK_NULL_HANDLE;

  std::vector<VkSemaphore> m_renderFinishedSemaphores;

  VkFence m_inFlightFence = VK_NULL_HANDLE;

  std::uint32_t m_currentImageIndex = 0;
  bool m_frameActive = false;

  VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
  VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
  VkSampler m_sampler = VK_NULL_HANDLE;
  std::vector<VulkanTextureResource> m_textureResources;
  TextureHandle m_fallbackTextureHandle = kInvalidTexture;
  VkSampler m_nearestSampler = VK_NULL_HANDLE;
};