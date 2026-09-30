#include "graphics/texturing.hpp"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>

DecodedImage TexLoader::LoadImageFromFile(const std::string &path) {
  DecodedImage result;

  SDL_Surface *loadedSurface = IMG_Load(path.c_str());
  if (!loadedSurface) {
    std::cerr << "[TextureLoader] ERROR: " << SDL_GetError() << "\n";
    return result;
  }

  SDL_Surface *formatted =
      SDL_ConvertSurface(loadedSurface, SDL_PIXELFORMAT_RGBA32);
  SDL_DestroySurface(loadedSurface);
  if (!formatted) {
    std::cerr << "[TextureLoader] ERROR: Failed format conversion for " << path
              << "\n";
    return result;
  }

  result.width = formatted->w;
  result.height = formatted->h;
  const int pitch = formatted->pitch;

  // Flip rows: SDL surfaces are top-down, GL wants bottom-up.
  // (Vulkan doesn't need this flip — see note below.)
  result.pixels.resize(static_cast<size_t>(pitch) * result.height);
  auto *src = static_cast<unsigned char *>(formatted->pixels);
  for (int y = 0; y < result.height; ++y) {
    memcpy(&result.pixels[(result.height - 1 - y) * pitch], &src[y * pitch],
           pitch);
  }

  SDL_DestroySurface(formatted);
  result.valid = true;
  return result;
}