
#include "defines.h"
#include "logger.h"

#include "core/re_application.h"

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

  _settings->windowSettings.title = _settings->title;

  //state.platform.Initialize(&_settings->windowSettings);
  if (!rePlatform.Initialize(&_settings->windowSettings))
  {
    IceLogFatal("Ice Platform failed to initialize");
    return false;
  }

  if (!reRenderer.Initialize(&_settings->rendererSettings))
  {
    IceLogFatal("Ice Renderer failed to initialize");
    return false;
  }

  state.ClientInitialize();
  return true;
}

b8 reIceApplication::Update()
{
  IceLogInfo("===== reApplication Main Loop =====");

  auto start = std::chrono::steady_clock::now();
  auto end = start;
  auto microsecDelta = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  float deltaTime = 0.0f;
  const float microToSeconds = 0.000001f;
 
  while (rePlatform.Update())
  {
    state.ClientUpdate(deltaTime);

    ICE_ATTEMPT(reRenderer.Render());

    // Update timing
    {
      end = std::chrono::steady_clock::now();
      microsecDelta = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
      deltaTime = microsecDelta.count() * microToSeconds;
      start = end;
    }
  }

  return true;
}

b8 reIceApplication::Shutdown()
{
  IceLogInfo("===== reApplication Shutdown =====");
  state.ClientShutdown();

  reRenderer.Shutdown();
  rePlatform.Shutdown();
  return true;
}
