#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct Vertex {
  float x, y, z;
  float red, green, blue, alpha;
  float u, v;
};

// Opaque handles — each backend interprets these however it likes internally.
using MeshHandle = uint32_t;
using TextureHandle = uint32_t;
constexpr MeshHandle kInvalidMesh = 0;
constexpr TextureHandle kInvalidTexture = 0;

class IRenderer {
public:
  virtual ~IRenderer() = default;

  virtual void Initialize() = 0;
  virtual bool CreateShaderProgram(const std::string &vertexSource,
                                   const std::string &fragmentSource) = 0;

  virtual void BeginFrame() = 0;
  virtual void Clear(float red, float green, float blue, float alpha) = 0;
  virtual void UseShaderProgram() = 0;
  virtual void EndFrame() = 0;
  virtual void OnResize(int width, int height) = 0;
  virtual void Shutdown() = 0;

  // Resource upload — backend decides how (VAO/VBO for GL, VkBuffer+VMA for
  // Vulkan)
  virtual MeshHandle UploadMesh(const std::vector<Vertex> &vertices,
                                const std::vector<unsigned int> &indices) = 0;
  virtual void DestroyMesh(MeshHandle handle) = 0;

  virtual TextureHandle UploadTexture(const unsigned char *pixels, int width,
                                      int height) = 0;
  virtual void DestroyTexture(TextureHandle handle) = 0;

  // Draw a previously-uploaded mesh, optionally bound to a texture (0 =
  // fallback)
  virtual void DrawMesh(MeshHandle mesh, TextureHandle texture) = 0;
};