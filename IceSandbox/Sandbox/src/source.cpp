
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

  //u32 blank = app.CreateMaterial({ {"blank", Ice_shader_VertFrag} });
  u32 def = app.CreateMaterial({ {"deferred", Ice_shader_VertFrag} });

  app.AddObject("Plane.obj", def);
  IceObject* obj = app.AddObject("Cube.obj", def);
  app.AddObject("SphereSmooth.obj", def, obj);
}

float pitch = 0.0f, yaw = 0.0f;
float zFar = 10.0f;
float zNear = 0.1f;
float halfWidth = 10.0f;

void reUpdate(float _deltaTime)
{
  if (Input.IsMouseButtonDown(Ice_Mouse_Right) || Input.IsKeyDown(Ice_Key_R))
  {
    i32 x, y;
    Input.GetMouseDelta(&x, &y);
    pitch += y;
    yaw += x;

    glm::mat4 view = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, -3.0f));
    view = glm::rotate(view, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    view = glm::rotate(view, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1280.0f / 720.0f, 0.01f, 1000.0f);
    // Not doing this incorrectly renders geometry, does not just flip the final image
    proj[1][1] *= -1; // Flip the projection's Y
    app.cam.viewProjectionMatrix = proj * view;
  }

  if (Input.OnKeyPressed(Ice_Key_P))
  {
    IceLogDebug("Reloading materials");
    reRenderer.ReloadMaterials();
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
  IceApplicationSettings settings = {};
  settings.title = "Ice";
  settings.version = 0;
  settings.ClientInitialize = reInit;
  settings.ClientUpdate = reUpdate;
  settings.ClientShutdown = reShutdown;

  settings.windowSettings.extents = { 1280, 720 };
  //settings.windowSettings.extents = { 1024, 1024 };
  settings.windowSettings.screenPosition = { 250, 150 };

  settings.rendererSettings.api = Ice_Renderer_Vulkan;

  return app.Run(&settings);
}
