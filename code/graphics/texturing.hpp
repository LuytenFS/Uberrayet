/*
 *   Copyright (C) - Adara, all rights reserved.
 */

#pragma once

#include <glad/gl.h>
#include <string>

GLuint CreateMissingTexture();

/* class that loads textures, pretty useful */
class TexLoader{
public:
    static GLuint LoadTextureFromFile(const std::string& path);

    static GLuint GetFallbackTexture();
private:
    static GLuint CreateMissingTexture();
};
