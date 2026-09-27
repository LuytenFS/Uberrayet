#include "vulkan_renderer.hpp"

VulkanRenderer::VulkanRenderer(SDL_Window *window, bool enableValidation)
    : m_window(window), m_enableValidation(enableValidation) {}

void VulkanRenderer::Initialize() {
  // TODO: vk-bootstrap instance/device/swapchain setup
}
bool VulkanRenderer::CreateShaderProgram(const std::string &,
                                         const std::string &) {
  // TODO: compile SPIR-V, build pipeline
  return false;
}
void VulkanRenderer::BeginFrame() {}
void VulkanRenderer::Clear(float, float, float, float) {}
void VulkanRenderer::UseShaderProgram() {}
void VulkanRenderer::EndFrame() {}
void VulkanRenderer::OnResize(int, int) {}
void VulkanRenderer::Shutdown() {}