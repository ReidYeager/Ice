
#include <ice.h>
#include <math\vector.h>
#include <core\input.h>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <chrono>
#include <iostream>
#include <math.h>

reIceApplication app;

void reInit()
{
  IceLogInfo("Client Init");

  u32 material = app.CreateMaterial({ {"blank", Ice_Shader_Vertex | Ice_Shader_Fragment} });
  u32 rainbow =  app.CreateMaterial({ {"blank", Ice_Shader_Vertex},
                                      {"rainbow", Ice_Shader_Fragment} });

  app.AddObject("BadCactus.obj", material);
  app.AddObject("Sphere.obj", rainbow);
}

float pitch = 0.0f, yaw = 0.0f;

void reUpdate(float _deltaTime)
{
  //IceLogInfo("Re-Update : %f", _deltaTime);
  if (Input.OnKeyPressed(Ice_Key_K))
  {
    IceLogInfo("KAY HOORAY");
  }

  if (Input.IsMouseButtonDown(Ice_Mouse_Right) || Input.IsMouseButtonDown(Ice_Mouse_Left))
  {
    i32 x, y;
    Input.GetMouseDelta(&x, &y);
    pitch += y;
    yaw += x;

    glm::mat4& viewProj = app.cam.viewProjectionMatrix;

    viewProj = glm::mat4(1);
    viewProj = glm::perspective(glm::radians(90.0f), 800.0f / 600.0f, 0.01f, 1000.0f);
    viewProj[1][1] *= -1; // Account for Vulkan's inverted Y screen coord
    viewProj = glm::translate(viewProj, glm::vec3(0.0f, 0.0f, -3.0f));
    viewProj = glm::rotate(viewProj, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    viewProj = glm::rotate(viewProj, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
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
