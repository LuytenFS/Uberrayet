/*
 *   Copyright (C) - Adara, all rights reserved.
 */

#include "mesh.hpp"
#include "texturing.hpp"

#include <utility>

Mesh::Mesh(const std::vector<Vertex> &vertices,
           const std::vector<unsigned int> &indices,
           const std::vector<Texture> &textures) {
  Setup(vertices, indices, textures);
}

Mesh::~Mesh() { Destroy(); }

Mesh::Mesh(Mesh &&other) noexcept
    : m_vao(other.m_vao), m_vbo(other.m_vbo), m_ebo(other.m_ebo),
      m_indexCount(other.m_indexCount),
      m_textures(std::move(other.m_textures)) {
  other.m_vao = 0;
  other.m_vbo = 0;
  other.m_ebo = 0;
  other.m_indexCount = 0;
}

Mesh &Mesh::operator=(Mesh &&other) noexcept {
  if (this != &other) {
    Destroy();
    m_vao = other.m_vao;
    m_vbo = other.m_vbo;
    m_ebo = other.m_ebo;
    m_indexCount = other.m_indexCount;
    m_textures = std::move(other.m_textures);

    other.m_vao = 0;
    other.m_vbo = 0;
    other.m_ebo = 0;
    other.m_indexCount = 0;
  }
  return *this;
}

void Mesh::Setup(const std::vector<Vertex> &vertices,
                 const std::vector<unsigned int> &indices,
                 const std::vector<Texture> &textures) {
  m_indexCount = static_cast<GLsizei>(indices.size());
  m_textures = textures;

  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);
  glGenBuffers(1, &m_ebo);

  glBindVertexArray(m_vao);

  // Upload Vertices
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
               vertices.data(), GL_STATIC_DRAW);

  // Upload Indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               indices.data(), GL_STATIC_DRAW);

  // Attribute 0: Position (3 floats: x, y, z)
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, x));

  // Attribute 1: Color (4 floats: red, green, blue, alpha)
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, red));

  // Attribute 2: TexCoords (2 floats: u, v)
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, u));

  glBindVertexArray(0);
}

void Mesh::Draw() const {
  if (m_vao != 0) {
    glActiveTexture(GL_TEXTURE0);

    if (!m_textures.empty() && m_textures[0].id != 0) {
      glBindTexture(GL_TEXTURE_2D, m_textures[0].id);
    } else {
      // Automatic fallback if no texture was provided
      glBindTexture(GL_TEXTURE_2D, TexLoader::GetFallbackTexture());
    }

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
}
void Mesh::Destroy() {
  if (m_vao != 0) {
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
    m_vao = 0;
    m_vbo = 0;
    m_ebo = 0;
    m_indexCount = 0;
    m_textures.clear();
  }
}