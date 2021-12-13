
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

  u32 light =  app.CreateMaterial({ {"blank", Ice_Shader_Vertex},
                                    {"lights", Ice_Shader_Fragment} });
  //u32 blank = app.CreateMaterial({ {"blank", Ice_shader_VertFrag} });
  u32 fresnel = app.CreateMaterial({ {"blank", Ice_Shader_Vertex},
                                     {"shadow", Ice_Shader_Fragment}});

  app.AddObject("BadCactus.obj", fresnel);
  app.AddObject("Plane.obj", fresnel);
}

float pitch = 0.0f, yaw = 0.0f;
float zFar = 10.0f;
float zNear = 0.1f;
float halfWidth = 10.0f;

void reUpdate(float _deltaTime)
{
  //if (Input.IsKeyDown(Ice_Key_R))
  //{
  //  static float tmpTime = 0.0f;
  //  reRenderer.backend.tmpLights.directionalDirection.x = sin(tmpTime);
  //  reRenderer.backend.tmpLights.directionalDirection.z = cos(tmpTime);

  //  tmpTime += _deltaTime * 2.0f;
  //}

  if (Input.IsKeyDown(Ice_Key_O))
  {
    zFar += 0.1f;
    IceLogInfo("--HWidth: %f -- zNear: %f -- zFar: %f", halfWidth, zNear, zFar);
  }
  if (Input.IsKeyDown(Ice_Key_L))
  {
    zFar -= 0.1f;
    IceLogInfo("--HWidth: %f -- zNear: %f -- zFar: %f", halfWidth, zNear, zFar);
  }

  if (Input.IsKeyDown(Ice_Key_I))
  {
    zNear += 0.1f;
    IceLogInfo("--HWidth: %f -- zNear: %f -- zFar: %f", halfWidth, zNear, zFar);
  }
  if (Input.IsKeyDown(Ice_Key_K))
  {
    zNear -= 0.1f;
    IceLogInfo("--HWidth: %f -- zNear: %f -- zFar: %f", halfWidth, zNear, zFar);
  }

  if (Input.IsKeyDown(Ice_Key_U))
  {
    halfWidth += 0.1f;
    IceLogInfo("--HWidth: %f -- zNear: %f -- zFar: %f", halfWidth, zNear, zFar);
  }
  if (Input.IsKeyDown(Ice_Key_J))
  {
    halfWidth -= 0.1f;
    IceLogInfo("--HWidth: %f -- zNear: %f -- zFar: %f", halfWidth, zNear, zFar);
  }


  if (Input.IsMouseButtonDown(Ice_Mouse_Right) || Input.IsMouseButtonDown(Ice_Mouse_Left))
  {
    i32 x, y;
    Input.GetMouseDelta(&x, &y);
    pitch += y;
    yaw += x;

    glm::mat4& viewProj = app.cam.viewProjectionMatrix;

    //viewProj = glm::mat4(1);
    //viewProj = glm::ortho(-halfWidth, halfWidth, -halfWidth, halfWidth, zNear, zFar);
    //viewProj[1][1] *= -1; // Account for Vulkan's inverted Y screen coord
    //glm::mat4 view = glm::lookAt(glm::vec3(20.0f, 20.0f, 20.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    //viewProj = viewProj * view;

    viewProj = glm::perspective(glm::radians(90.0f), 1280.0f / 720.0f, 0.01f, 1000.0f);
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

  settings.windowSettings.extents = { 1280, 720 };
  //settings.windowSettings.extents = { 1024, 1024 };
  settings.windowSettings.screenPosition = { 100, 50 };

  settings.rendererSettings.api = Ice_Renderer_Vulkan;

  return app.Run(&settings);
}
