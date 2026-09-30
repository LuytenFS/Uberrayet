#pragma once

#include <string>
#include <cstdint>

namespace uberrayet 
{
    enum class RendererBackend{
        OpenGL,
        Vulkan
    };

    struct CommandLineOptions
    {
        RendererBackend renderer         = RendererBackend::OpenGL;
        bool            vulkanValidation = false;
        bool            fullscreen       = false;
        uint32_t        windowWidth      = 1280;
        uint32_t        windowHeight     = 720;
        bool            showHelp         = false;

        bool            valid            = true;
    };

    class CommandLine {
    public:
        // Parses argv[1..argc-1]. argv[0] (program name) is ignored for parsing
        // but used in help text.
        static CommandLineOptions Parse(int argc, char **argv);

        static void PrintHelp(const char* programName);

    private:
        static bool ParseRenderer(const std::string& value, RendererBackend& out);
    };
} // uberrayet

