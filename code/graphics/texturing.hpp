#pragma once

#include <string>
#include <vector>

struct DecodedImage {
  std::vector<unsigned char> pixels; // RGBA8, top-to-bottom
  int width = 0;
  int height = 0;
  bool valid = false;
};

class TexLoader {
public:
  static DecodedImage LoadImageFromFile(const std::string &path);
};