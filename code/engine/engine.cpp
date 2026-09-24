#include "engine.hpp"
#include "graphics/renderer.hpp"
#include "graphics/texturing.hpp"
#include "utilities/externals.hpp"
#include "utilities/logging.hpp"

#include <SDL3/SDL.h>
#include <cstdio>
#include <glad/gl.h>
#include <iostream>

void Engine::Initialize() {
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
    m_logger.Error(
        "Engine Initialization Failed",
        (std::string("SDL_Init failed: ") + SDL_GetError()).c_str());
    return;
  }

  // Configure OpenGL 4.6 Core Profile attributes
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  SDL_WindowFlags windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
  m_window = SDL_CreateWindow("Uberrayet", m_screenWidth, m_screenHeight, windowFlags);

  if (!m_window) {
    m_logger.Error(
        "Engine Initialization Failed",
        (std::string("SDL_CreateWindow failed: ") + SDL_GetError()).c_str());
    SDL_Quit();
    return;
  }
  
  SDL_SetWindowRelativeMouseMode(m_window, true);
  
  m_glContext = SDL_GL_CreateContext(m_window);
  if (!m_glContext) {
    m_logger.Error("Engine Initialization Failed",
                 (std::string("SDL_GL_CreateContext failed: ") + SDL_GetError())
                     .c_str());
    SDL_DestroyWindow(m_window);
    m_window = nullptr;
    SDL_Quit();
    return;
  }

  if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
    m_logger.Error("Engine Initialization Failed",
                 "glad failed to load GL functions.");
    SDL_GL_DestroyContext(m_glContext);
    SDL_DestroyWindow(m_window);
    m_glContext = nullptr;
    m_window = nullptr;
    SDL_Quit();
    return;
  }

  std::printf("OpenGL context: %s\n", glGetString(GL_VERSION));
  SDL_GL_SetSwapInterval(m_vsync ? 1 : 0);

  // Set initial viewport based on window dimensions
  glViewport(0, 0, m_screenWidth, m_screenHeight);
  m_renderer.Initialize();
  m_isRunning = true;
}

void Engine::LoadContent() {
  std::string vertexCode = LoadShaderSource("assets/shaders/basic.vert");
  std::string fragmentCode = LoadShaderSource("assets/shaders/basic.frag");

  if (!m_renderer.CreateShaderProgram(vertexCode, fragmentCode)) {
    std::cerr << "Failed to build shader program in LoadContent!\n";
  }

  std::vector<Vertex> squareVertices = {
      //  x,      y,     z,    red,  green, blue, alpha,  u,    v
      {-0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f}, // 0: Top-Left
      {0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},  // 1: Top-Right
      {0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
       0.0f}, // 2: Bottom-Right
      {-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f} // 3: Bottom-Left
  };

  // Must contain 6 indices for 2 triangles
  std::vector<unsigned int> squareIndices = {
      0, 1, 2, // First Triangle  (Top-Left -> Top-Right -> Bottom-Right)
      0, 2, 3  // Second Triangle (Top-Left -> Bottom-Right -> Bottom-Left)
  };

  std::vector<unsigned int> triangleIndices = {0, 1, 2};

  Texture containerTex;
  containerTex.id = TexLoader::LoadTextureFromFile("assets/textures/container.png");
  containerTex.type = "texture_diffuse";
  containerTex.path = "assets/textures/container.png";
  std::vector<Texture> textures = {containerTex};

  // Upload mesh data with textures attached
  m_triangleMesh.Setup(squareVertices, squareIndices, textures);
}
void Engine::Run() {
  Initialize();
  if (!m_isRunning)
    return;

  LoadContent();

  Uint64 lastTime = SDL_GetTicksNS();
  constexpr float fixedStep = 1.0f / 60.0f; // 60 Hz fixed physics step
  float accumulator = 0.0f;

  while (m_isRunning) {
    Uint64 currentTime = SDL_GetTicksNS();
    m_deltaTime = static_cast<float>(currentTime - lastTime) / 1e9f;
    lastTime = currentTime;

    // Cap maximum frame time to avoid physics collapse on lag spikes
    if (m_deltaTime > 0.25f) {
      m_deltaTime = 0.25f;
    }

    accumulator += m_deltaTime;

    ProcessInput();

    while (accumulator >= fixedStep) {
      FixedUpdate(fixedStep);
      accumulator -= fixedStep;
    }

    Update(m_deltaTime);
    Draw();
  }

  UnloadContent();
  Cleanup();
}

void Engine::ProcessInput() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT) {
      Quit();
    }
    if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
      Quit();
    }
    if (event.type == SDL_EVENT_WINDOW_RESIZED) {
      m_screenWidth = event.window.data1;
      m_screenHeight = event.window.data2;
      glViewport(0, 0, m_screenWidth, m_screenHeight);
    }

    m_mouse.Handle_Event(event); // feed motion events to Mouse
  }

  m_keyboard.Update(); // refresh scancode snapshot + previous-frame diff
  m_mouse.End_Frame(); // refresh button state, reset per-frame delta accumulator
}

void Engine::Update(float deltaTime) {
  // Update game states, animations, timers
}

void Engine::FixedUpdate(float fixedDeltaTime) {
  // Perform physics simulations, collision checks
}

void Engine::Draw() {
  m_renderer.Clear(0.02f, 0.02f, 0.06f, 1.0f);

  // Bind shader program
  glUseProgram(m_renderer.GetShaderProgram());

  // Draw triangle mesh
  m_triangleMesh.Draw();

  SDL_GL_SwapWindow(m_window);
}

void Engine::UnloadContent() { m_triangleMesh.Destroy(); }

void Engine::Cleanup() {
  m_renderer.Shutdown();

  if (m_glContext) {
    SDL_GL_DestroyContext(m_glContext);
    m_glContext = nullptr;
  }
  if (m_window) {
    SDL_DestroyWindow(m_window);
    m_window = nullptr;
  }
  SDL_Quit();
}