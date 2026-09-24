#pragma once

#include <SDL3/SDL.h>
#include <source_location>
#include <string>

class Logger {
public:
  void Assert(const char *title, const char *message,
              std::source_location loc = std::source_location::current());
  void Error(const char *title, const char *message,
             std::source_location loc = std::source_location::current());
  void Warning(const char *title, const char *message,
               std::source_location loc = std::source_location::current());

private:
  // Returns the clicked button id, or -1 on failure/close.
  int ShowBox(SDL_MessageBoxFlags flags, const char *title,
              const std::string &message,
              const SDL_MessageBoxButtonData *buttons, int numButtons);
};