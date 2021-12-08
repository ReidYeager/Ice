
#include <iostream>
#include <ice.h>

#include <math\vector.h>

#include <math.h>
#include <chrono>

reIceApplication app;

void reInit()
{
  IceLogInfo("Client Init");
}

void reUpdate(float _deltaTime)
{
  //IceLogInfo("Re-Update : %f", _deltaTime);
}

void reShutdown()
{
  IceLogInfo("Client Shutdown");
}


int main()
{
  reIceApplicationSettings settings = {};
  settings.title = "reIce";
  settings.version = 0;
  settings.ClientInitialize = reInit;
  settings.ClientUpdate = reUpdate;
  settings.ClientShutdown = reShutdown;

  settings.windowSettings.extents = { 800, 600 };
  settings.windowSettings.screenPosition = { 100, 50 };

  settings.rendererSettings.api = Ice_Renderer_Vulkan;

  return app.Run(&settings);
}
