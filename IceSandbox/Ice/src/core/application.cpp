
#include "defines.h"
#include "logger.h"

#include "core/application.h"
#include "core/input.h"

#include <chrono>

u32 reIceApplication::Run(reIceApplicationSettings* _settings)
{
  try
  {
    if (!Initialize(_settings))
    {
      IceLogFatal("IceApplication Initialization failed");
      return -1;
    }

    if (!Update())
    {
      IceLogFatal("IceApplication Update failed");
      return -2;
    }

    if (!Shutdown())
    {
      IceLogFatal("IceApplication Shutdown failed");
      return -3;
    }

    return 0;
  }
  catch (const char* error)
  {
    IceLogFatal("Ice caught :: %s", error);
    return -4;
  }
}

b8 reIceApplication::Initialize(reIceApplicationSettings* _settings)
{
  state.ClientInitialize = _settings->ClientInitialize;
  state.ClientUpdate = _settings->ClientUpdate;
  state.ClientShutdown = _settings->ClientShutdown;

  IceLogInfo("===== reApplication Initialize =====");

  // Initialize the platform =====
  _settings->windowSettings.title = _settings->title;
  if (!rePlatform.Initialize(&_settings->windowSettings))
  {
    IceLogFatal("Ice Platform failed to initialize");
    return false;
  }
  Input.Initialize();

  // Initialize the renderer =====
  if (!reRenderer.Initialize(&_settings->rendererSettings))
  {
    IceLogFatal("Ice Renderer failed to initialize");
    return false;
  }

  // Set camera default state =====
  glm::mat4 viewProj = glm::mat4(1);
  viewProj = glm::perspective(glm::radians(90.0f), 1280.0f / 720.0f, 0.01f, 1000.0f);
  viewProj[1][1] *= -1; // Account for Vulkan's inverted Y screen coord
  viewProj = glm::translate(viewProj, glm::vec3(0.0f, 0.0f, -3.0f));
  cam.viewProjectionMatrix = viewProj;

  // Initialize the client game =====
  state.ClientInitialize();

  return true;
}

float averageSum = 0.0f;
u32 averageCount = 0;

b8 reIceApplication::Update()
{
  IceLogInfo("===== reApplication Main Loop =====");

  auto start = std::chrono::steady_clock::now();
  auto end = start;
  auto microsecDelta = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  float deltaTime = 0.0f;
  const float microToMilli = 0.001f;
  const float microToSecond = microToMilli * 0.001f;

  float deltaSum = 0.0f;
  u32 deltaCount = 0;
 
  while (rePlatform.Update())
  {
    state.ClientUpdate(deltaTime);

    ICE_ATTEMPT(reRenderer.Render(&cam));

    // Update timing =====
    {
      end = std::chrono::steady_clock::now();
      microsecDelta = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
      deltaTime = microsecDelta.count() * microToSecond;
      start = end;

      // Timing log =====
      deltaSum += deltaTime;
      deltaCount++;
      if (deltaSum >= 1.0f)
      {
        float averageSec = (deltaSum / deltaCount);
        IceLogInfo("%3.3f ms -- %3.0f FPS", averageSec * 1000.0f, 1.0f / averageSec);

        averageSum += averageSec;
        averageCount++;

        deltaSum = 0;
        deltaCount = 0;
      }

      // Update input =====
      Input.Update();
    }
  }

  return true;
}

b8 reIceApplication::Shutdown()
{
  float totalAverageDelta = averageSum / averageCount;
  IceLogInfo("Average delta : %3.3f ms -- %3.0f FPS",
             totalAverageDelta * 1000.0f,
             1.0f / totalAverageDelta);

  IceLogInfo("===== reApplication Shutdown =====");
  state.ClientShutdown();

  reRenderer.Shutdown();
  rePlatform.Shutdown();
  return true;
}

void reIceApplication::AddObject(const char* _meshDir, u32 _material)
{
  u32 meshIndex = reRenderer.CreateMesh(_meshDir);
  reRenderer.AddMeshToScene(meshIndex, _material);
}

u32 reIceApplication::CreateMaterial(std::vector<IceShaderInfo> _shaders)
{
  return reRenderer.CreateMaterial(_shaders);
}
