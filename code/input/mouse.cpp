#include "mouse.hpp"

void Mouse::Handle_Event(const SDL_Event &e) {
  if (e.type == SDL_EVENT_MOUSE_MOTION) {
    dx += e.motion.xrel;
    dy += e.motion.yrel;
  }
}

void Mouse::End_Frame() {
  dx = 0.0f;
  dy = 0.0f;
  buttons = SDL_GetMouseState(nullptr, nullptr);
}

bool Mouse::Is_Down(Uint8 button) const {
  return buttons & SDL_BUTTON_MASK(button);
}
