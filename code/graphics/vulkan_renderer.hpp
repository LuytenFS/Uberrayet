#pragma once

#include "irenderer.hpp"

struct SDL_Window;

class VulkanRenderer : public IRenderer {
public:
  VulkanRenderer(SDL_Window *window, bool enableValidation);

  void Initialize() override;
  bool CreateShaderProgram(const std::string &vertexSource,
                           const std::string &fragmentSource) override;
  void BeginFrame() override;
  void Clear(float red, float green, float blue, float alpha) override;
  void UseShaderProgram() override;
  void EndFrame() override;
  void OnResize(int width, int height) override;
  void Shutdown() override;
  
  MeshHandle UploadMesh(const std::vector<Vertex> &,
                        const std::vector<unsigned int> &) override {
    return kInvalidMesh;
  }
  void DestroyMesh(MeshHandle) override {}
  TextureHandle UploadTexture(const unsigned char *, int, int) override {
    return kInvalidTexture;
  }
  void DestroyTexture(TextureHandle) override {}
  void DrawMesh(MeshHandle, TextureHandle) override {}

private:
  SDL_Window *m_window = nullptr;
  bool m_enableValidation = false;
  // instance / device / swapchain members go here once you write bring-up
};