#include "logging.hpp"
#include <array>
#include <cstdio>
#include <string>

int Logger::ShowBox(SDL_MessageBoxFlags flags, const char *title,
                    const std::string &message,
                    const SDL_MessageBoxButtonData *buttons, int numButtons) {
  SDL_MessageBoxData box{};
  box.flags = flags;
  box.window = nullptr;
  box.title = title;
  box.message = message.c_str();
  box.numbuttons = numButtons;
  box.buttons = buttons;

  int buttonId = -1;
  if (!SDL_ShowMessageBox(&box, &buttonId)) {
    std::fprintf(stderr, "SDL_ShowMessageBox failed: %s\n", SDL_GetError());
  }
  return buttonId;
}

void Logger::Assert(const char *title, const char *message,
                    std::source_location loc) {
  // Programmer/logic error — debug-facing, offers a debugger break.
  std::string full_message = std::string(message) +
                             "\n\nFile: " + loc.file_name() +
                             "\nLine: " + std::to_string(loc.line());

  const std::array<SDL_MessageBoxButtonData, 3> buttons{{
      {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Abort"},
      {0, 1, "Debug"},
      {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 2, "Ignore"},
  }};

  int buttonId = ShowBox(SDL_MESSAGEBOX_ERROR, title, full_message,
                         buttons.data(), static_cast<int>(buttons.size()));

  switch (buttonId) {
  case 0:
    std::abort();
  case 1:
    SDL_TriggerBreakpoint();
    break;
  case 2:
  default:
    break;
  }
}

void Logger::Error(const char *title, const char *message,
                   std::source_location loc) {
  // Fatal, user-facing (release + debug) — no Debug button, this isn't a
  // code-invariant check.
  std::string full_message = std::string(message) +
                             "\n\nFile: " + loc.file_name() +
                             "\nLine: " + std::to_string(loc.line());

  const std::array<SDL_MessageBoxButtonData, 2> buttons{{
      {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Quit"},
      {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "Ignore"},
  }};

  int buttonId = ShowBox(SDL_MESSAGEBOX_ERROR, title, full_message,
                         buttons.data(), static_cast<int>(buttons.size()));

  if (buttonId == 0) {
    std::abort();
  }
  // buttonId == 1 or -1 → continue execution
}

void Logger::Warning(const char *title, const char *message,
                     std::source_location loc) {
  // Non-fatal, no abort path at all — just informs and continues.
  std::string full_message = std::string(message) +
                             "\n\nFile: " + loc.file_name() +
                             "\nLine: " + std::to_string(loc.line());

  const std::array<SDL_MessageBoxButtonData, 1> buttons{{
      {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "OK"},
  }};

  ShowBox(SDL_MESSAGEBOX_WARNING, title, full_message, buttons.data(),
          static_cast<int>(buttons.size()));
}