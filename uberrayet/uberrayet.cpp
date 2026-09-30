#include "code/engine/engine.hpp"
#include "code/internals/cmdline.hpp"

int main(int argc, char **argv) {
  const auto options = uberrayet::CommandLine::Parse(argc, argv);

  if (options.showHelp) {
    uberrayet::CommandLine::PrintHelp(argv[0]);
    return 0;
  }

  if (!options.valid) {
    uberrayet::CommandLine::PrintHelp(argv[0]);
    return 1;
  }

  Engine engine(options.windowWidth, options.windowHeight, 60, 0.0f, true,
                options.renderer, options.vulkanValidation);
  engine.Run();
  return 0;
}