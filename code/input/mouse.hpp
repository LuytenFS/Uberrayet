#pragma once
#include <SDL3/SDL.h>

class Mouse {
public:
  void Handle_Event(const SDL_Event &e); // call from your event poll loop
  void End_Frame(); // call once per frame, after input processed

  float delta_x() const { return dx; }
  float delta_y() const { return dy; }
  bool Is_Down(Uint8 button) const;

private:
  float dx = 0.0f, dy = 0.0f;
  Uint32 buttons = 0;
};
