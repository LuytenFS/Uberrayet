#pragma once

#include "code/graphics/irenderer.hpp"
#include "code/input/keyboard.hpp"
#include "code/input/mouse.hpp"
#include "internals/cmdline.hpp"
#include "utilities/logging.hpp"
#include <SDL3/SDL.h>
#include <memory>

class Engine {
public:
  Engine() = default;
  Engine(
      int screenWidth, int screenHeight, int framerate, float deltaTime,
      bool vsync,
      uberrayet::RendererBackend backend = uberrayet::RendererBackend::OpenGL,
      bool vulkanValidation = false)
      : m_screenWidth(screenWidth), m_screenHeight(screenHeight),
        m_framerate(framerate), m_deltaTime(deltaTime), m_vsync(vsync),
        m_backend(backend), m_vulkanValidation(vulkanValidation) {}

  void Initialize();
  bool LoadContent();
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
  bool InitializeOpenGL();
  bool InitializeVulkan();

  bool m_isRunning = false;

  uberrayet::RendererBackend m_backend = uberrayet::RendererBackend::OpenGL;
  bool m_vulkanValidation = false;

  SDL_Window *m_window = nullptr;
  SDL_GLContext m_glContext = nullptr; // only set when m_backend == OpenGL

  std::unique_ptr<IRenderer> m_renderer;

  MeshHandle m_triangleMeshHandle = kInvalidMesh;
  TextureHandle m_containerTextureHandle = kInvalidTexture;

  Keyboard m_keyboard;
  Mouse m_mouse;
  Logger m_logger;
};