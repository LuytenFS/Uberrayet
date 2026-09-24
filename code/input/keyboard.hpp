#pragma once
#include "SDL3/SDL_scancode.h"
#include <SDL3/SDL.h>

class Keyboard{
public:
    void Update();
    bool Is_Down(SDL_Scancode key) const;
    bool Just_Pressed(SDL_Scancode key) const;
    bool Just_Released(SDL_Scancode key) const;
private:
    const bool* state = nullptr;
    int num_keys = 0;
    bool previous[SDL_SCANCODE_COUNT] = { false };
};
