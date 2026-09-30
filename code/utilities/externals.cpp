#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

std::string LoadShaderSource(const std::string &filePath) {
  std::ifstream file(filePath, std::ios::binary);

  if (!file.is_open()) {
    std::cerr << "Failed to open shader file: " << filePath << '\n';
    return {};
  }

  std::ostringstream buffer;
  buffer << file.rdbuf();

  return buffer.str();
}