
#include <ice.h>
#include <math\vector.h>
#include <core\input.h>

#include <chrono>
#include <iostream>
#include <math.h>

reIceApplication app;

void reInit()
{
  IceLogInfo("Client Init");

  //iceMaterial mat = app.GetShader({
  //    { "blank", Ice_Shader_Vertex },
  //    { "red", Ice_Shader_Fragment }
  //    });

}

void reUpdate(float _deltaTime)
{
  //IceLogInfo("Re-Update : %f", _deltaTime);
  if (Input.OnKeyPressed(Ice_Key_K))
  {
    IceLogInfo("KAY HOORAY");
  }

  if (Input.OnKeyPressed(Ice_Key_Escape))
  {
    rePlatform.Close();
  }
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
