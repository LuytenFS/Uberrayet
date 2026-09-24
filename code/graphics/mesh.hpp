/*
 *   Copyright (C) - Adara, all rights reserved.
 */

#include <glad/gl.h>
#include <vector>
#include <string>

struct Vertex{
    GLfloat x, y, z; // Coordinates
    float red, green, blue, alpha;
    float u, v;
};

struct Texture{
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh{
public:
    Mesh() = default;
    Mesh(const std::vector<Vertex> &vertices,
         const std::vector<unsigned int> &indices,
         const std::vector<Texture> &textures = {});
    ~Mesh();

    /* prevent accidental copying to avoid double-deleting OpenGL Buffers*/
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    /* allow move semantics */
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    void Setup(const std::vector<Vertex> &vertices,
               const std::vector<unsigned int> &indices,
               const std::vector<Texture> &textures = {});
    void Draw() const;
    void Destroy();
private:
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;
    GLsizei m_indexCount = 0;
    std::vector<Texture> m_textures;
};
