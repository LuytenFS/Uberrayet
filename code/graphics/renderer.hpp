/*
 *   Copyright (C) - Adara, all rights reserved.
 */

#pragma once

#include <glad/gl.h>
#include <string>

class Renderer{
public:
    void Initialize();
    bool CreateShaderProgram(const std::string &vertexSource,
                             const std::string &fragmentSource);
    void BeginFrame();
    void Clear(float red, float green, float blue, float alpha);
    void Draw3DMesh();
    void Draw2DMesh();
    void EndFrame();
    void Shutdown();

    [[nodiscard]] GLuint GetShaderProgram() const { return m_shaderProgram; }

private:
    GLuint m_shaderProgram = 0;
};
