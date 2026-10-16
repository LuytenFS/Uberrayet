#include "opengl_renderer.hpp"
#include "utilities/logging.hpp"
#include "utilities/externals.hpp"

#include <glad/gl.h>
#include <string>

bool OpenGLRenderer::Initialize() {
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  return true;
}

bool OpenGLRenderer::CreateShaderProgram(const std::string &vertexPath,
                                         const std::string &fragmentPath) {
  const std::string vertexSource = LoadShaderSource(vertexPath);
  const std::string fragmentSource = LoadShaderSource(fragmentPath);

  if (vertexSource.empty() || fragmentSource.empty()) {
    m_logger.Error(
        "OpenGLRenderer",
        "Cannot create shader program: failed to load shader source file(s).");
    return false;
  }

  const char *vSource = vertexSource.c_str();
  const char *fSource = fragmentSource.c_str();

  // Compile Vertex Shader
  GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertShader, 1, &vSource, nullptr);
  glCompileShader(vertShader);

  GLint success = 0;
  glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetShaderInfoLog(vertShader, 512, nullptr, infoLog);
    m_logger.Error(
        "OpenGLRenderer",
        (std::string("Vertex Shader Compilation Error: ") + infoLog).c_str());
    glDeleteShader(vertShader);
    return false;
  }

  // Compile Fragment Shader
  GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragShader, 1, &fSource, nullptr);
  glCompileShader(fragShader);

  glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetShaderInfoLog(fragShader, 512, nullptr, infoLog);
    m_logger.Error(
        "OpenGLRenderer",
        (std::string("Fragment Shader Compilation Error: ") + infoLog).c_str());
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    return false;
  }

  // Link Shader Program
  m_shaderProgram = glCreateProgram();
  glAttachShader(m_shaderProgram, vertShader);
  glAttachShader(m_shaderProgram, fragShader);
  glLinkProgram(m_shaderProgram);

  glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetProgramInfoLog(m_shaderProgram, 512, nullptr, infoLog);
    m_logger.Error(
        "OpenGLRenderer",
        (std::string("Shader Program Linking Error: ") + infoLog).c_str());
  }

  // Clean up shader objects post-linking
  glDeleteShader(vertShader);
  glDeleteShader(fragShader);

  m_mvpLoc = glGetUniformLocation(m_shaderProgram, "u_mvp");
  return success != 0;
}

void OpenGLRenderer::BeginFrame()
{

}

void OpenGLRenderer::Clear(float red, float green, float blue, float alpha) 
{
  glClearColor(red, green, blue, alpha);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::UseShaderProgram() { glUseProgram(m_shaderProgram); }

void OpenGLRenderer::EndFrame()
{

}

void OpenGLRenderer::OnResize(int width, int height) {
  glViewport(0, 0, width, height);
}

void OpenGLRenderer::Shutdown() {
  if (m_shaderProgram != 0) {
    glDeleteProgram(m_shaderProgram);
    m_shaderProgram = 0;
  }
}

MeshHandle
OpenGLRenderer::UploadMesh(const std::vector<Vertex> &vertices,
                           const std::vector<unsigned int> &indices) {
  GLMesh m;
  m.indexCount = static_cast<GLsizei>(indices.size());

  glGenVertexArrays(1, &m.vao);
  glGenBuffers(1, &m.vbo);
  glGenBuffers(1, &m.ebo);

  glBindVertexArray(m.vao);
  glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
               vertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               indices.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, x));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, red));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, u));
  glBindVertexArray(0);

  const MeshHandle handle = m_nextMeshHandle++;
  m_meshes[handle] = m;
  return handle;
}

void OpenGLRenderer::DestroyMesh(MeshHandle handle) {
  auto it = m_meshes.find(handle);
  if (it == m_meshes.end())
    return;
  glDeleteVertexArrays(1, &it->second.vao);
  glDeleteBuffers(1, &it->second.vbo);
  glDeleteBuffers(1, &it->second.ebo);
  m_meshes.erase(it);
}

TextureHandle OpenGLRenderer::UploadTexture(const unsigned char *pixels,
                                            int width, int height) {
  GLuint id;
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, pixels);
  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);

  const TextureHandle handle = m_nextTextureHandle++;
  m_textures[handle] = id;
  return handle;
}

void OpenGLRenderer::DestroyTexture(TextureHandle handle) {
  auto it = m_textures.find(handle);
  if (it == m_textures.end())
    return;
  glDeleteTextures(1, &it->second);
  m_textures.erase(it);
}

void OpenGLRenderer::DrawMesh(MeshHandle mesh, TextureHandle texture,
                              const mat4 &mvp) {
  auto meshIt = m_meshes.find(mesh);
  if (meshIt == m_meshes.end())
    return;

  glUniformMatrix4fv(m_mvpLoc, 1, GL_FALSE, &mvp.data[0]);

  glActiveTexture(GL_TEXTURE0);
  auto texIt = m_textures.find(texture);
  if (texIt != m_textures.end()) {
    glBindTexture(GL_TEXTURE_2D, texIt->second);
  } else {
    if (m_fallbackTexture == 0) {
      unsigned char pixels[] = {255, 0, 255, 255, 0,   0, 0,   255,
                                0,   0, 0,   255, 255, 0, 255, 255};

      glGenTextures(1, &m_fallbackTexture);
      glBindTexture(GL_TEXTURE_2D, m_fallbackTexture);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA,
                   GL_UNSIGNED_BYTE, pixels);
      glBindTexture(GL_TEXTURE_2D, 0);
    }
    glBindTexture(GL_TEXTURE_2D, m_fallbackTexture);
  }

  glBindVertexArray(meshIt->second.vao);
  glDrawElements(GL_TRIANGLES, meshIt->second.indexCount, GL_UNSIGNED_INT,
                 nullptr);
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
}