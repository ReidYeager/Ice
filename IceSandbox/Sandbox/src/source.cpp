
#include <ice.h>
#include <math\vector.h>
#include <core\input.h>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <chrono>
#include <iostream>
#include <math.h>

reIceApplication app;

u32 albedoLight;
u32 shadowLight;
b8 usingAlbedoLight;
u32 blank;

void reInit()
{
  IceLogInfo("Client Init");

  albedoLight = app.CreateLightingMaterial({ { "_light_blank", Ice_Shader_Vertex },
                                             { "_light_pbr", Ice_Shader_Fragment } });
  shadowLight = app.CreateLightingMaterial({ { "_light_blank", Ice_Shader_Vertex },
                                             { "_light_shadow", Ice_Shader_Fragment } });

  if (!app.SetLightingMaterial(albedoLight))
    return;
  usingAlbedoLight = true;

  blank = app.CreateMaterial(Ice_Mat_Deferred,
                             { {"blank_deferred", Ice_Shader_Vertex},
                               {"blank_deferred", Ice_Shader_Fragment} });

  u32 pbr = app.CreateMaterial(Ice_Mat_Deferred,
                               { {"pbr", Ice_Shader_Vertex},
                                 {"pbr", Ice_Shader_Fragment} });

  app.AssignMaterialTextures(pbr,
                             {{"TestAlbedo.png", Ice_Image_Color},  // Albedo
                              {"TestNormal.png", Ice_Image_Normal}, // Normal
                              {"PixelBlack.png", Ice_Image_Map},    // Metallic
                              {"PixelBlack.png", Ice_Image_Map},    // Roughness
                              {"PixelWhite.png", Ice_Image_Map}     // AO
                              });

  u32 gun = app.CreateMaterial(Ice_Mat_Deferred,
                               { {"pbr", Ice_Shader_Vertex},
                                 {"pbr", Ice_Shader_Fragment} });

  app.AssignMaterialTextures(gun,
                             {{"CyborgWeapon/Weapon_albedo.png", Ice_Image_Color},  // Albedo
                              {"CyborgWeapon/Weapon_normal.png", Ice_Image_Normal}, // Normal
                              {"CyborgWeapon/Weapon_metallic.png", Ice_Image_Map},  // Metallic
                              {"CyborgWeapon/Weapon_roughness.png", Ice_Image_Map}, // Roughness
                              {"CyborgWeapon/Weapon_ao.png", Ice_Image_Map}         // AO
                              });

  u32 fwdMat = app.CreateMaterial(Ice_Mat_Forward,
                                  { {"blank_forward", Ice_Shader_Vertex},
                                    {"blank_forward", Ice_Shader_Fragment} });

  app.AddObject("Plane.obj", pbr);
  //app.AddObject("BadCactus.obj", blank);
  //app.AddObject("Sphere.obj", blank);
  //app.AddObject("Cube.obj", pbr);
  IceObject* g = app.AddObject("Cyborg_Weapon.obj", gun);

  g->transform.scale = {4.0f, 4.0f, 4.0f};
  g->transform.UpdateMatrix();
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
    proj[1][1] *= -1;
    app.cam.viewProjectionMatrix = proj * view;
  }

  if (Input.OnKeyPressed(Ice_Key_P))
  {
    IceLogDebug("Reloading materials");
    if (!renderer.ReloadMaterials())
    {
      throw "Failed to reload materials";
    }
  }

  if (Input.OnKeyPressed(Ice_Key_L))
  {
    IceLogDebug("Switching lighting material to : %s", usingAlbedoLight ? "shadow" : "albedo");
    if (app.SetLightingMaterial(usingAlbedoLight ? shadowLight : albedoLight))
    {
      usingAlbedoLight = !usingAlbedoLight;
    }
  }

  vec4 value = { (sin(app.totalTime) * 0.5f) + 0.5f, 0.0f, 0.0f, 0.0f};
  app.SetMaterialBufferData(blank, &value);

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
  settings.windowSettings.screenPosition = { 200, 75 };

  settings.rendererSettings.api = Ice_Renderer_Vulkan;

  return app.Run(&settings);
}
