#include "graphics/texturing.hpp"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <glad/gl.h>
#include <iostream>
#include <vector>

GLuint TexLoader::CreateMissingTexture() {
  GLuint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);

  // Set texture wrapping parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // Set texture filtering parameters (Nearest keeps the pixelated grid crisp)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // 2x2 pixels: [Magenta, Black, Black, Magenta] (RGBA format)
  unsigned char pixels[] = {255, 0, 255, 255, 0,   0, 0,   255,
                            0,   0, 0,   255, 255, 0, 255, 255};

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               pixels);
  glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0);
  return textureID;
}

GLuint TexLoader::GetFallbackTexture()
{
    static GLuint fallbackID = CreateMissingTexture();
    return fallbackID;
}

GLuint TexLoader::LoadTextureFromFile(const std::string &path) {
  // 1. Load surface via SDL_image
  SDL_Surface *loadedSurface = IMG_Load(path.c_str());
  if (!loadedSurface) {
    std::cerr << "[TextureLoader] ERROR: " << SDL_GetError()
              << " -> Falling back to missing texture.\n";
    return GetFallbackTexture();
  }

  // 2. Convert to standard RGBA format if needed
  SDL_Surface *formattedSurface =
      SDL_ConvertSurface(loadedSurface, SDL_PIXELFORMAT_RGBA32);
  SDL_DestroySurface(loadedSurface);

  if (!formattedSurface) {
    std::cerr << "[TextureLoader] ERROR: Failed format conversion for " << path
              << " -> Falling back.\n";
    return GetFallbackTexture();
  }

  // 3. Flip pixel rows vertically for OpenGL Y-axis alignment
  int width = formattedSurface->w;
  int height = formattedSurface->h;
  int pitch = formattedSurface->pitch;

  std::vector<unsigned char> flippedPixels(pitch * height);
  unsigned char *srcPixels =
      static_cast<unsigned char *>(formattedSurface->pixels);

  for (int y = 0; y < height; ++y) {
    // Copy rows from bottom to top
    memcpy(&flippedPixels[(height - 1 - y) * pitch], &srcPixels[y * pitch],
           pitch);
  }

  // 4. Generate OpenGL Texture
  GLuint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, flippedPixels.data());
  glGenerateMipmap(GL_TEXTURE_2D);

  // Free the surface memory
  SDL_DestroySurface(formattedSurface);
  glBindTexture(GL_TEXTURE_2D, 0);

  return textureID;
}