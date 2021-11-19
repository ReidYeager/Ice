
#include "defines.h"
#include "logger.h"

#include "core/re_application.h"

#include <chrono>

void reIceApplication::Run(reIceApplicationSettings* _settings)
{
  try
  {
    if (!Initialize(_settings))
    {
      IceLogFatal("IceApplication Initialization failed");
      return;
    }

    if (!Update())
    {
      IceLogFatal("IceApplication Update failed");
      return;
    }

    if (!Shutdown())
    {
      IceLogFatal("IceApplication Shutdown failed");
      return;
    }
  }
  catch (const char* error)
  {
    IceLogFatal("Ice caught :: %s", error);
  }
}

b8 reIceApplication::Initialize(reIceApplicationSettings* _settings)
{
  state.ClientInitialize = _settings->ClientInitialize;
  state.ClientUpdate = _settings->ClientUpdate;
  state.ClientShutdown = _settings->ClientShutdown;

  IceLogInfo("reApplication Initialize");

  //state.platform.Initialize(&_settings->windowSettings);
  rePlatform.Initialize(&_settings->windowSettings);

  // TODO : ~!!~ Initialize renderer

  state.ClientInitialize();

  return true;
}

b8 reIceApplication::Update()
{
  auto start = std::chrono::steady_clock::now();
  auto end = start;
  auto microsecDelta = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  float deltaTime = 0.0f;
  const float microToSeconds = 0.000001f;
 
 while (rePlatform.Update())
  {
    state.ClientUpdate(deltaTime);

    // Update timimg
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
  state.ClientShutdown();

  rePlatform.Shutdown();

  IceLogInfo("reApplication Shutdown");
  return true;
}
