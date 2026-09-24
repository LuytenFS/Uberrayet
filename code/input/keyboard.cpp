#include "keyboard.hpp"
#include "SDL3/SDL_keyboard.h"
#include "SDL3/SDL_scancode.h"
#include <cstring>

void Keyboard::Update()
{
    std::memcpy(previous, state ? state : previous, num_keys);
    state = SDL_GetKeyboardState(&num_keys);
}

bool Keyboard::Is_Down(SDL_Scancode key) const {
    return state && state[key];
}

bool Keyboard::Just_Pressed(SDL_Scancode key) const {
    return state && state[key] && !previous[key];
}

bool Keyboard::Just_Released(SDL_Scancode key) const {
  return state && !state[key] && previous[key];
}
