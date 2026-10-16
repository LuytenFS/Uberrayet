#include "engine.hpp"
#include "graphics/opengl_renderer.hpp"
#include "graphics/texturing.hpp"
#include "graphics/vulkan_renderer.hpp"
#include "utilities/logging.hpp"
#include "math/umath.hpp"

#include <SDL3/SDL.h>
#include <cstdio>
#include <glad/gl.h>
#include <iostream>
#include <algorithm>

bool Engine::InitializeOpenGL() {
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  SDL_WindowFlags windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
  m_window =
      SDL_CreateWindow("Uberrayet", m_screenWidth, m_screenHeight, windowFlags);
  if (!m_window) {
    m_logger.Error(
        "Engine Initialization Failed",
        (std::string("SDL_CreateWindow failed: ") + SDL_GetError()).c_str());
    return false;
  }

  SDL_SetWindowRelativeMouseMode(m_window, true);

  m_glContext = SDL_GL_CreateContext(m_window);
  if (!m_glContext) {
    m_logger.Error(
        "Engine Initialization Failed",
        (std::string("SDL_GL_CreateContext failed: ") + SDL_GetError())
            .c_str());
    SDL_DestroyWindow(m_window);
    m_window = nullptr;
    return false;
  }

  if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
    m_logger.Error("Engine Initialization Failed",
                   "glad failed to load GL functions.");
    SDL_GL_DestroyContext(m_glContext);
    SDL_DestroyWindow(m_window);
    m_glContext = nullptr;
    m_window = nullptr;
    return false;
  }

  std::printf("OpenGL context: %s\n", glGetString(GL_VERSION));
  SDL_GL_SetSwapInterval(m_vsync ? 1 : 0);

  m_renderer = std::make_unique<OpenGLRenderer>();
  return true;
}

bool Engine::InitializeVulkan() {
  SDL_WindowFlags windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
  m_window =
      SDL_CreateWindow("Uberrayet", m_screenWidth, m_screenHeight, windowFlags);
  if (!m_window) {
    m_logger.Error(
        "Engine Initialization Failed",
        (std::string("SDL_CreateWindow failed: ") + SDL_GetError()).c_str());
    return false;
  }

  SDL_SetWindowRelativeMouseMode(m_window, true);

  m_renderer = std::make_unique<VulkanRenderer>(m_window, m_vulkanValidation);
  return true;
}

void Engine::Initialize() {
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
    m_logger.Error("Engine Initialization Failed",
                   (std::string("SDL_Init failed: ") + SDL_GetError()).c_str());
    return;
  }

  const bool backendReady = (m_backend == uberrayet::RendererBackend::Vulkan)
                                ? InitializeVulkan()
                                : InitializeOpenGL();
  if (!backendReady) {
    SDL_Quit();
    return;
  }
  if (!m_renderer->Initialize()) {
    m_logger.Error("Engine Initialization Failed",
                   "Renderer::Initialize() failed.");
    Cleanup();
    return; // m_isRunning stays false, Run() will bail before LoadContent
  }
  m_renderer->OnResize(m_screenWidth, m_screenHeight);
  m_camera.SetAspect(static_cast<float>(m_screenWidth) /
                     static_cast<float>(std::max(m_screenHeight, 1)));
  m_isRunning = true;
}

bool Engine::LoadContent() {
  if (!m_renderer->CreateShaderProgram("assets/shaders/basic.vert",
                                       "assets/shaders/basic.frag")) {
    std::cerr << "Failed to build shader program\n";
    return false;
  }

  const std::vector<Vertex> vertices = {
      {-0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f},
      {0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
      {0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f},
      {-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f}};

  const std::vector<unsigned int> indices = {0, 1, 2, 0, 2, 3};

  m_triangleMeshHandle = m_renderer->UploadMesh(vertices, indices);

  if (m_triangleMeshHandle == kInvalidMesh) {
    std::cerr << "Failed to upload mesh\n";
    return false;
  }

  DecodedImage img =
      TexLoader::LoadImageFromFile("assets/textures/container.png");
  if (img.valid) {
    m_containerTextureHandle =
        m_renderer->UploadTexture(img.pixels.data(), img.width, img.height);
    if (m_containerTextureHandle == kInvalidTexture) {
      std::cerr
          << "Failed to upload texture (renderer returned invalid handle)\n";
    }
  } else {
    std::cerr << "Failed to load container.png\n";
    m_containerTextureHandle = kInvalidTexture;
  }

  return true;
}
void Engine::Run() {
  Initialize();
  if (!m_isRunning)
    return;

  if (!LoadContent()) {
    Cleanup();
    return;
  }

  Uint64 lastTime = SDL_GetTicksNS();
  constexpr float fixedStep = 1.0f / 60.0f;
  float accumulator = 0.0f;

  while (m_isRunning) {
    Uint64 currentTime = SDL_GetTicksNS();
    m_deltaTime = static_cast<float>(currentTime - lastTime) / 1e9f;
    lastTime = currentTime;
    if (m_deltaTime > 0.25f)
      m_deltaTime = 0.25f;

    accumulator += m_deltaTime;
    ProcessInput();
    while (accumulator >= fixedStep) {
      FixedUpdate(fixedStep);
      accumulator -= fixedStep;
    }
    Update(m_deltaTime);
    Draw();
    m_mouse.End_Frame();
  }

  UnloadContent();
  Cleanup();
}

void Engine::ProcessInput() {
  SDL_Event event{};

  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT) {
      Quit();
    }

    if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
      Quit();
    }

    if (event.type == SDL_EVENT_WINDOW_RESIZED ||
        event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
      int pixelWidth = 0;
      int pixelHeight = 0;

      SDL_GetWindowSizeInPixels(m_window, &pixelWidth, &pixelHeight);

      m_screenWidth = pixelWidth;
      m_screenHeight = pixelHeight;
      m_camera.SetAspect(static_cast<float>(pixelWidth) /
                         static_cast<float>(std::max(pixelHeight, 1)));

      m_renderer->OnResize(pixelWidth, pixelHeight);
    }

    m_mouse.Handle_Event(event);
  }

  m_keyboard.Update();
}

void Engine::Update(float dt) {
  constexpr float sens = 0.0025f;    // radians per pixel
  constexpr float rollSpeed = 1.5f;  // rad/s
  constexpr double moveSpeed = 10.0; // units/s

  // 6DOF: deltas are applied in the camera's LOCAL frame
  quat yaw = quat_from_axis_angle(0, 1, 0, -m_mouse.delta_x() * sens);
  quat pitch = quat_from_axis_angle(1, 0, 0, -m_mouse.delta_y() * sens);
  m_camera.Rotate(yaw);
  m_camera.Rotate(pitch);

  float roll = 0.0f;
  if (m_keyboard.Is_Down(SDL_SCANCODE_Q))
    roll += rollSpeed * dt;
  if (m_keyboard.Is_Down(SDL_SCANCODE_E))
    roll -= rollSpeed * dt;
  if (roll != 0.0f)
    m_camera.Rotate(quat_from_axis_angle(0, 0, 1, roll));

  double r = 0, u = 0, f = 0;
  if (m_keyboard.Is_Down(SDL_SCANCODE_W))
    f += 1;
  if (m_keyboard.Is_Down(SDL_SCANCODE_S))
    f -= 1;
  if (m_keyboard.Is_Down(SDL_SCANCODE_D))
    r += 1;
  if (m_keyboard.Is_Down(SDL_SCANCODE_A))
    r -= 1;
  if (m_keyboard.Is_Down(SDL_SCANCODE_SPACE))
    u += 1;
  if (m_keyboard.Is_Down(SDL_SCANCODE_LSHIFT))
    u -= 1;
  m_camera.MoveLocal(r * moveSpeed * dt, u * moveSpeed * dt,
                     f * moveSpeed * dt);
}

void Engine::FixedUpdate(float fixedDeltaTime) {

}

void Engine::Draw() {
  const bool vk = (m_backend == uberrayet::RendererBackend::Vulkan);

  const mat4 view = m_camera.GetViewMatrix();
  const mat4 proj = vk ? m_camera.GetProjectionMatrixVulkan()
                       : m_camera.GetProjectionMatrixOpenGL();

  double cx, cy, cz;
  m_camera.GetPosition(cx, cy, cz);

  // Camera-relative: subtract in double precision, THEN cast to float
  const mat4 model = mat4_translation(static_cast<float>(m_quadX - cx),
                                      static_cast<float>(m_quadY - cy),
                                      static_cast<float>(m_quadZ - cz));
  const mat4 mvp = mat4_multiply(proj, mat4_multiply(view, model));

  m_renderer->BeginFrame();
  m_renderer->Clear(0.02f, 0.02f, 0.06f, 1.0f);
  m_renderer->UseShaderProgram();

  m_renderer->DrawMesh(m_triangleMeshHandle, m_containerTextureHandle, mvp);

  m_renderer->EndFrame();

  if (m_backend == uberrayet::RendererBackend::OpenGL) {
    SDL_GL_SwapWindow(m_window); // Vulkan will present via swapchain instead
  }
}

void Engine::UnloadContent() {
  m_renderer->DestroyMesh(m_triangleMeshHandle);
  m_renderer->DestroyTexture(m_containerTextureHandle);
}

void Engine::Cleanup() {
  if (m_renderer) {
    m_renderer->Shutdown();
    m_renderer.reset();
  }

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