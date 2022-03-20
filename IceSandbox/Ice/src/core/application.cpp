
#include "defines.h"
#include "logger.h"

#include "core/application.h"

#include "core/input.h"
#include "core/ecs.h"
#include "platform/platform.h"
#include "rendering/vulkan/vulkan.h"

#include <chrono>

b8(*GameUpdateFunc)();
b8(*GameShutdownFunc)();
Ice::Renderer* renderer;

//=========================
// Time
//=========================

Ice::IceTime Ice::time;
// Setup time =====
std::chrono::steady_clock::time_point realtimeStart, frameStart, frameEnd;
const float microToSecond = 0.000001f;

void InitTime()
{
  Ice::time.deltaTime = 0.0f;
  Ice::time.totalTime = 0.0f;
  Ice::time.frameCount = 0;

  realtimeStart = std::chrono::steady_clock::now();
  frameStart = frameEnd = realtimeStart;

  Ice::time.deltaTime = 0.0f;
}

void UpdateTime()
{
  frameEnd = std::chrono::steady_clock::now();

  Ice::time.deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - frameStart).count() * microToSecond;
  Ice::time.totalTime = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - realtimeStart).count() * microToSecond;

  Ice::time.frameCount++;

  frameStart = frameEnd;
}

//=========================
// Application
//=========================

b8 IceApplicationInitialize(Ice::ApplicationSettings _settings)
{
  // Platform =====
  if (!Ice::platform.CreateNewWindow(_settings.window))
  {
    IceLogFatal("Failed to initialize the renderer");
    return false;
  }
  Input.Initialize();

  // Rendering =====
  if (_settings.renderer.api == Ice::Renderer_Vulkan)
  {
    renderer = new Ice::RendererVulkan();
  }
  else
  {
    IceLogFatal("Selected API not supported");
    return false;
  }

  if (!renderer->Init(_settings.renderer))
  {
    IceLogFatal("Failed to initialize the renderer");
    return false;
  }

  // Game =====
  _settings.clientInitFunction();
  GameUpdateFunc = _settings.clientUpdateFunction;
  GameShutdownFunc = _settings.clientShutdownFunction;

  return true;
}

b8 IceApplicationUpdate()
{
  InitTime();

  while (Ice::platform.Update())
  {
    ICE_ATTEMPT_BOOL(GameUpdateFunc());

    ICE_ATTEMPT_BOOL(renderer->RenderFrame());

    Input.Update();
    UpdateTime();
  }

  return true;
}

b8 IceApplicationShutdown()
{
  ICE_ATTEMPT_BOOL(GameShutdownFunc());

  renderer->Shutdown();

  Input.Shutdown();
  Ice::CloseWindow();
  Ice::platform.Shutdown();

  return true;
}

u32 Ice::Run(ApplicationSettings _settings)
{
  if (!IceApplicationInitialize(_settings))
  {
    IceLogFatal("Ice application initialization failed");
    return -1;
  }

  if (!IceApplicationUpdate())
  {
    IceLogFatal("Ice application update failed");
    return -2;
  }

  if (!IceApplicationShutdown())
  {
    IceLogFatal("Ice application shutdown failed");
    return -3;
  }

  return 0;
}

void Ice::CloseWindow()
{
  platform.CloseWindow();
}

//=========================
// Rendering
//=========================

std::vector<Ice::Material> materials;
//Ice::ECS::ComponentManager<Ice::Material> materials;

Ice::Material Ice::CreateMaterial(Ice::MaterialSettings _settings)
{
  // Don't search for existing materials to allow one material setup with multiple buffer values
  // TODO : Create buffers for multiple instances of a material (instead of creating multiple materials)

  Ice::Material newMaterial = renderer->CreateMaterial(_settings);

  materials.push_back(newMaterial);

  return newMaterial;
}
