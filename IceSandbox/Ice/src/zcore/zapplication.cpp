
#include "defines.h"
#include "logger.h"

#include "zcore/zapplication.h"
#include "core/input.h"

#include <chrono>

Ice::IceTime Ice::time;
b8(*gameUpdate)() = 0;
b8(*gameShutdown)() = 0;
zIceWindow window;

b8 Init(Ice::zIceApplicationSettings _settings)
{
  Input.Initialize();

  window = Ice::CreateNewWindow(_settings.windowSettings);

  //if (!renderer.Initialize(_settings.rendererSettings))
  //{
  //  IceLogFatal("Failed to initialize renderer");
  //}

  ICE_ATTEMPT_BOOL(_settings.gameInit());
  gameUpdate = _settings.gameUpdate;
  gameShutdown = _settings.gameShutdown;

  Ice::InitTime();

  return true;
}

b8 Update()
{
  float avgDelta = 0.0f;
  float deltaSum = 0.0f;
  u32 frameCount = 0;

  while (window.Update())
  {
    ICE_ATTEMPT_BOOL(gameUpdate());

    Input.Update();
    Ice::UpdateTime();

    // Debug timing log =====
    deltaSum += Ice::time.deltaTime;
    frameCount++;
    if (deltaSum >= 1.0f)
    {
      avgDelta = deltaSum / frameCount;
      IceLogDebug("%u frames : %5.3f ms : %f FPS\n",
                  Ice::time.frameCount,
                  avgDelta * 1000.0f,
                  1.0f / avgDelta);

      deltaSum = 0.0f;
      frameCount = 0;
    }
  }

  return true;
}

b8 Shutdown()
{
  ICE_ATTEMPT_BOOL(gameShutdown());

  window.Close();

  int* x = (int*)Ice::MemoryAllocate(sizeof(int));
  IceLogInfo("%p : %d", x, *x);
  Ice::MemoryZero(x, sizeof(int));
  IceLogInfo("%p : %d", x, *x);
  Ice::MemoryFree(x);
  IceLogInfo("%p : %d", x, *x);

  return true;
}

b8 Ice::Run(zIceApplicationSettings _settings)
{
  try
  {
    if (!Init(_settings))
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
