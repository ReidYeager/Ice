
#include "defines.h"
#include "logger.h"

#include "core/application.h"

#include "core/input.h"
#include "core/ecs.h"
#include "platform/platform.h"
#include "rendering/vulkan/vulkan.h"

#include <chrono>

Ice::ApplicationSettings settings;
Ice::Renderer* renderer;
b8 isRunning;

// Rendering =====
u32 shaderCount = 0;
Ice::Shader* shaders;
u32 materialCount = 0;
Ice::Material* materials;
//Ice::ECS::ComponentManager<Ice::Material> materials;

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
  settings = _settings;

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

  shaders = (Ice::Shader*)Ice::MemoryAllocZero(sizeof(Ice::Shader) * _settings.maxShaderCount);
  materials = (Ice::Material*)Ice::MemoryAllocZero(sizeof(Ice::Material) * _settings.maxMaterialCount);

  // Game =====
  _settings.clientInitFunction();

  return true;
}

b8 IceApplicationUpdate()
{
  InitTime();

  while (isRunning && Ice::platform.Update())
  {
    ICE_ATTEMPT_BOOL(settings.clientUpdateFunction(Ice::time.deltaTime));

    ICE_ATTEMPT_BOOL(renderer->RenderFrame());

    Input.Update();
    UpdateTime();
  }

  return true;
}

b8 IceApplicationShutdown()
{
  ICE_ATTEMPT_BOOL(settings.clientShutdownFunction());

  for (u32 i = 0; i < materialCount; i++)
  {
    renderer->DestroyMaterial(materials[i]);
  }
  Ice::MemoryFree(materials);

  for (u32 i = 0; i < shaderCount; i++)
  {
    renderer->DestroyShader(shaders[i]);
  }
  Ice::MemoryFree(shaders);

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

  isRunning = true;

  if (!IceApplicationUpdate())
  {
    IceLogFatal("Ice application update failed");
    return -2;
  }

  isRunning = false;

  if (!IceApplicationShutdown())
  {
    IceLogFatal("Ice application shutdown failed");
    return -3;
  }

  return 0;
}

void Ice::Shutdown()
{
  isRunning = false;
}

void Ice::CloseWindow()
{
  platform.CloseWindow();
}

//=========================
// Rendering
//=========================

Ice::Material& Ice::CreateMaterial(Ice::MaterialSettings _settings)
{
  // Don't search for existing materials to allow one material setup with multiple buffer values
  // TODO : Create buffers for multiple instances of a material (instead of creating multiple materials)

  if (materialCount == settings.maxMaterialCount)
  {
    IceLogError("Maximum material count reached");
    return materials[0];
  }

  // Get shaders' info =====
  b8 shaderFound = false;
  for (u32 i = 0; i < _settings.shaders.size(); i++)
  {
    shaderFound = false;
    Ice::Shader& newShader = _settings.shaders[i];

    // Check for existing shader
    for (u32 j = 0; j < shaderCount; j++)
    {
      Ice::Shader& oldShader = shaders[j];
      if (newShader.fileDirectory.compare(oldShader.fileDirectory) == 0 &&
        newShader.type == oldShader.type)
      {
        shaderFound = true;
        newShader = oldShader;
        break;
      }
    }

    // Create new shader
    if (!shaderFound)
    {
      if (shaderCount == settings.maxShaderCount)
      {
        IceLogError("Maximum shader count reached");
        return materials[0];
      }

      newShader = renderer->CreateShader(newShader);
      shaders[shaderCount] = newShader;
      shaderCount++;
    }
  }

  // Create material =====
  Ice::Material newMaterial = renderer->CreateMaterial(_settings);
  materials[materialCount] = newMaterial;
  materialCount++;

  return materials[materialCount - 1];
}
