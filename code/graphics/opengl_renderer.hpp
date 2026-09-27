#pragma once

#include "irenderer.hpp"
#include "utilities/logging.hpp"

#include <glad/gl.h>
#include <string>
#include <unordered_map>

class OpenGLRenderer : public IRenderer {
public:
  void Initialize() override;
  bool CreateShaderProgram(const std::string &vertexSource,
                           const std::string &fragmentSource) override;
  void BeginFrame() override;
  void Clear(float red, float green, float blue, float alpha) override;
  void UseShaderProgram() override;
  void EndFrame() override;
  void OnResize(int width, int height) override;
  void Shutdown() override;

  [[nodiscard]] GLuint GetShaderProgram() const { return m_shaderProgram; }

  MeshHandle UploadMesh(const std::vector<Vertex> &vertices,
                        const std::vector<unsigned int> &indices) override;
  void DestroyMesh(MeshHandle handle) override;
  TextureHandle UploadTexture(const unsigned char *pixels, int width,
                              int height) override;
  void DestroyTexture(TextureHandle handle) override;
  void DrawMesh(MeshHandle mesh, TextureHandle texture) override;

private:
  GLuint m_shaderProgram = 0;
  Logger m_logger;
  struct GLMesh {
    GLuint vao = 0, vbo = 0, ebo = 0;
    GLsizei indexCount = 0;
  };
  std::unordered_map<MeshHandle, GLMesh> m_meshes;
  std::unordered_map<TextureHandle, GLuint> m_textures;
  MeshHandle m_nextMeshHandle = 1;
  TextureHandle m_nextTextureHandle = 1;
  GLuint m_fallbackTexture = 0;
};