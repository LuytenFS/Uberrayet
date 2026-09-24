#include "renderer.hpp"
#include "utilities/logging.hpp"

#include <glad/gl.h>
#include <string>

void Renderer::Initialize()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

bool Renderer::CreateShaderProgram(const std::string &vertexSource,
                                   const std::string &fragmentSource) {
  if (vertexSource.empty() || fragmentSource.empty()) {
    m_logger.Error(
        "Renderer",
        "Cannot create shader program: empty source string provided.");
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
        "Renderer",
        (std::string("Vertex Shader Compilation Error: ") + infoLog).c_str());
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
        "Renderer",
        (std::string("Fragment Shader Compilation Error: ") + infoLog).c_str());
    glDeleteShader(vertShader);
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
        "Renderer",
        (std::string("Shader Program Linking Error: ") + infoLog).c_str());
  }

  // Clean up shader objects post-linking
  glDeleteShader(vertShader);
  glDeleteShader(fragShader);

  return success != 0;
}
void Renderer::EndFrame()
{

}

void Renderer::Clear(float red, float green, float blue, float alpha)
{
    glClearColor(red, green, blue, alpha);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::Draw2DMesh()
{

}

void Renderer::Draw3DMesh()
{

}

void Renderer::Shutdown() {
  if (m_shaderProgram != 0) {
    glDeleteProgram(m_shaderProgram);
    m_shaderProgram = 0;
  }
}