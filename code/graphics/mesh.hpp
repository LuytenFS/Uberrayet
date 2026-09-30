#pragma once

#include "irenderer.hpp"
#include <vector>

class Mesh {
public:
  Mesh() = default;

  void SetData(std::vector<Vertex> vertices,
               std::vector<unsigned int> indices) {
    m_vertices = std::move(vertices);
    m_indices = std::move(indices);
  }

  [[nodiscard]] const std::vector<Vertex> &GetVertices() const {
    return m_vertices;
  }
  [[nodiscard]] const std::vector<unsigned int> &GetIndices() const {
    return m_indices;
  }

private:
  std::vector<Vertex> m_vertices;
  std::vector<unsigned int> m_indices;
};