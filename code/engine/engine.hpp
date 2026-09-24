/*
 *   Copyright (C) - Adara, all rights reserved.
 */

#pragma once

#include "code/graphics/mesh.hpp"
#include "code/graphics/renderer.hpp"
#include <SDL3/SDL.h>

class Engine {
public:
  Engine() = default;
  Engine(int screenWidth, int screenHeight, int framerate, float deltaTime,
         bool vsync)
      : m_screenWidth(screenWidth), m_screenHeight(screenHeight),
        m_framerate(framerate), m_deltaTime(deltaTime), m_vsync(vsync) {}

  void Initialize();
  void LoadContent();
  void Run();

  void ProcessInput();
  void Update(float deltaTime);
  void FixedUpdate(float fixedDeltaTime);
  void Draw();

  void UnloadContent();
  void Cleanup();

  [[nodiscard]] bool IsRunning() const { return m_isRunning; }
  void Quit() { m_isRunning = false; }

  int m_screenWidth = 1280;
  int m_screenHeight = 720;
  int m_framerate = 60;
  float m_deltaTime = 0.0f;
  bool m_vsync = true;

private:
  bool m_isRunning = false;

  SDL_Window *m_window = nullptr;
  SDL_GLContext m_glContext = nullptr;
  Renderer m_renderer;
  Mesh m_triangleMesh;
};