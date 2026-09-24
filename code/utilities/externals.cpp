#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

std::string LoadShaderSource(const char *filePath) {
  std::ifstream file(filePath);
  if (!file.is_open()) {
    std::cerr << "Failed to open shader file: " << filePath << '\n';
    return "";
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}