
#include <stdio.h>
#include <ice.h>

#include "platform/compact_array.h"

#define SOURCE_DIR EXPAND_STRING_DEF(TESTAPP_DIRECTORY)

b8 Init();
b8 Update(f32 _delta);
b8 Shutdown();

int main()
{
  Ice::ApplicationSettings settings;
  settings.GameInit = Init;
  settings.GameUpdate = Update;
  settings.GameShutdown = Shutdown;
  settings.rendererCore.api = Ice::RenderApiVulkan;

  settings.window.extents = { 800, 600 };
  settings.window.position = { 50, 50 };
  settings.window.title = "Test title";

  Ice::Run(settings);
  return 0;
}

struct A { int x; int y; };
struct B { char c; };
struct C { f32 x; u64 f; };

Ice::Material* material;
Ice::Scene scene;
Ice::Entity cam;

b8 Init()
{
  Ice::TMPSetMainScene(&scene);

  Ice::CameraSettings camSettings {};
  camSettings.farClip = 100.0f;
  camSettings.nearClip = 0.1f;
  camSettings.isPerspective = true;
  camSettings.verticalFov = 45.0f;
  camSettings.ratio = 4.0f / 3.0f;

  cam = Ice::CreateCamera(&scene, camSettings);
  //scene.GetComponent<Ice::Transform>(cam)->SetPosition({0.0f, 1.5f, 3.0f});

  Ice::MaterialSettings materialSettings;
  Ice::ShaderSettings shaderInfo;
  shaderInfo.fileDirectory = SOURCE_DIR "/res/shaders/compiled/blank";
  shaderInfo.type = Ice::Shader_Vertex;
  materialSettings.shaderSettings.push_back(shaderInfo);
  shaderInfo.type = Ice::Shader_Fragment;
  materialSettings.shaderSettings.push_back(shaderInfo);

  ICE_ATTEMPT(Ice::CreateMaterial(materialSettings, &material));

  Ice::Entity gun = Ice::CreateRenderedEntity(&scene);
  Ice::RenderComponent* gunRC = scene.GetComponent<Ice::RenderComponent>(gun);
  gunRC->material = material;
  ICE_ATTEMPT(Ice::CreateMesh(SOURCE_DIR "/res/models/BadCactus.obj", &gunRC->mesh));

  Ice::Entity cube = Ice::CreateRenderedEntity(&scene);
  Ice::RenderComponent* cubeRC = scene.GetComponent<Ice::RenderComponent>(cube);
  cubeRC->material = material;
  ICE_ATTEMPT(Ice::CreateMesh(SOURCE_DIR "/res/models/Cube.obj", &cubeRC->mesh));

  return true;
}

b8 Update(f32 _delta)
{
  scene.GetComponent<Ice::Transform>(cam)->Translate({ 0.0f, 0.5f * _delta, 0.5f * _delta });
  scene.GetComponent<Ice::Transform>(cam)->RotateEuler({ -5.0f * _delta, 0.0f, 0.0f });

  if (Input.IsKeyDown(Ice_Key_Escape))
    return false;

  return true;
}

b8 Shutdown()
{
  return true;
}
