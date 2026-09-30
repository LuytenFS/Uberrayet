#include "cmdline.hpp"

#include <cstdint>
#include <iostream>
#include <cstdlib>
#include <cstring>

namespace uberrayet
{
// Splits "--key=value" into ("--key", "value"). Returns false if there's no '='.
bool SplitKeyValue(const std::string &arg, std::string &key,
                   std::string &value) {
  const auto pos = arg.find('=');
  if (pos == std::string::npos) {
    return false;
  }
  key = arg.substr(0, pos);
  value = arg.substr(pos + 1);
  return true;
}

bool ParseUInt(const std::string& value, uint32_t& out){
    if (value.empty()) return false;
    char* end = nullptr;
    const unsigned long parsed = std::strtoul(value.c_str(), &end, 10);
    if (end == value.c_str() || *end != '\0'){
        return false;
    }
    out = static_cast<uint32_t>(parsed);
    return true;
}
bool CommandLine::ParseRenderer(const std::string &value,
                                RendererBackend &out) {
  if (value == "gl" || value == "opengl") {
    out = RendererBackend::OpenGL;
    return true;
  }
  if (value == "vulkan" || value == "vk") {
    out = RendererBackend::Vulkan;
    return true;
  }
  return false;
}
CommandLineOptions CommandLine::Parse(int argc, char **argv) {
  CommandLineOptions options;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];

    if (arg == "-h" || arg == "--help") {
      options.showHelp = true;
      continue;
    }

    if (arg == "--vulkan-validation") {
      options.vulkanValidation = true;
      continue;
    }

    if (arg == "--fullscreen") {
      options.fullscreen = true;
      continue;
    }

    std::string key, value;
    if (SplitKeyValue(arg, key, value)) {
      if (key == "--renderer") {
        if (!ParseRenderer(value, options.renderer)) {
          std::cerr << "Unknown renderer '" << value
                    << "'. Expected 'gl' or 'vulkan'.\n";
          options.valid = false;
        }
        continue;
      }
      if (key == "--width") {
        if (!ParseUInt(value, options.windowWidth)) {
          std::cerr << "Invalid width '" << value << "'.\n";
          options.valid = false;
        }
        continue;
      }
      if (key == "--height") {
        if (!ParseUInt(value, options.windowHeight)) {
          std::cerr << "Invalid height '" << value << "'.\n";
          options.valid = false;
        }
        continue;
      }
    }

    std::cerr << "Unknown argument: " << arg << "\n";
    options.valid = false;
  }

  return options;
}

void CommandLine::PrintHelp(const char *programName) {
  std::cout
      << "Usage: " << programName
      << " [options]\n"
         "\n"
         "Options:\n"
         "  -h, --help                Show this help message and exit\n"
         "  --renderer=gl|vulkan      Select rendering backend (default: gl)\n"
         "  --vulkan-validation       Enable Vulkan validation layers\n"
         "  --fullscreen              Start in fullscreen mode\n"
         "  --width=<n>               Window width in pixels (default: 1280)\n"
         "  --height=<n>              Window height in pixels (default: 720)\n";
}
} // uberrayet

