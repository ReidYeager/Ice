
#include <ice.h>

// Used to print the average frame time for this session
f32 totalDeltaSum = 0.0;
u32 totalDeltaCount = 0;

Ice::Object* gun;
Ice::Object* sphere;
Ice::Object* camera;

b8 Init()
{
  Ice::MaterialSettings materialSettings;
  Ice::Shader shaderInfo;
  shaderInfo.fileDirectory = "blank";
  shaderInfo.type = Ice::Shader_Vertex;
  materialSettings.shaders.push_back(shaderInfo);
  shaderInfo.type = Ice::Shader_Fragment;
  materialSettings.shaders.push_back(shaderInfo);
  //lightMatSettings.subpass = 1;

  Ice::Material* lightMat;
  Ice::CreateMaterial(materialSettings, &lightMat);
  //Ice::SetLightingMaterial(lightMat);

  //Ice::MaterialSettings blankMatSettings { "blank_deferred", "blank_deferred" };
  //Ice::Material blank = Ice::CreateMaterial(blankMatSettings);

  gun = &Ice::CreateObject();
  Ice::AttatchRenderComponent(gun, "Cyborg_Weapon.obj", lightMat);
  gun->transform->rotation.y = 90.0f;

  sphere = &Ice::CreateObject();
  Ice::AttatchRenderComponent(sphere, "Sphere.obj", lightMat);

  sphere->transform->position.y = -1.0f;

  Ice::CameraSettings camSettings;
  camSettings.isProjection = true;
  camSettings.horizontal = 45.0f;
  camSettings.ratio = 800.0f / 600.0f;
  camSettings.nearClip = 0.01f;
  camSettings.farClip = 10.0f;

  camera = &Ice::CreateObject();
  Ice::AttatchCameraComponent(camera, camSettings);
  camera->transform->position.z = 3.0f;

  return true;
}

b8 Update(f32 _delta)
{
  if (Input.OnKeyPressed(Ice_Key_Escape))
  {
    Ice::Shutdown();
  }
  if (Input.OnKeyPressed(Ice_Key_P))
  {
    IceLogInfo("Should re-load shaders & re-create material pipelines");
  }

  const f32 degreesPerSecond = 90.0f;

  //gun->transform->position.x = cos(Ice::time.totalTime);
  gun->transform->rotation.x += degreesPerSecond * _delta * Input.IsKeyDown(Ice_Key_I);
  gun->transform->rotation.x -= degreesPerSecond * _delta * Input.IsKeyDown(Ice_Key_K);

  gun->transform->rotation.y += degreesPerSecond * _delta * Input.IsKeyDown(Ice_Key_J);
  gun->transform->rotation.y -= degreesPerSecond * _delta * Input.IsKeyDown(Ice_Key_L);

  gun->transform->rotation.z += degreesPerSecond * _delta * Input.IsKeyDown(Ice_Key_U);
  gun->transform->rotation.z -= degreesPerSecond * _delta * Input.IsKeyDown(Ice_Key_O);


  gun->transform->position.x += _delta * Input.IsKeyDown(Ice_Key_D);
  gun->transform->position.x -= _delta * Input.IsKeyDown(Ice_Key_A);

  gun->transform->position.y += _delta * Input.IsKeyDown(Ice_Key_E);
  gun->transform->position.y -= _delta * Input.IsKeyDown(Ice_Key_Q);

  gun->transform->position.z += _delta * Input.IsKeyDown(Ice_Key_S);
  gun->transform->position.z -= _delta * Input.IsKeyDown(Ice_Key_W);

  gun->transform->scale.x += _delta * Input.IsKeyDown(Ice_Key_H);
  gun->transform->scale.x -= _delta * Input.IsKeyDown(Ice_Key_F);

  gun->transform->scale.y += _delta * Input.IsKeyDown(Ice_Key_T);
  gun->transform->scale.y -= _delta * Input.IsKeyDown(Ice_Key_G);

  gun->transform->scale.z += _delta * Input.IsKeyDown(Ice_Key_Y);
  gun->transform->scale.z -= _delta * Input.IsKeyDown(Ice_Key_R);

  camera->transform->position.x = cos(Ice::time.totalTime) * 2.5f;
  camera->transform->position.z = sin(Ice::time.totalTime) * 2.5f;
  camera->transform->rotation.y = (-Ice::time.totalTime * 57.2958279088f) + 90.0f;

  #ifdef ICE_DEBUG
  // Log the average delta time & framerate =====
  static f32 deltasSum = 0.0f;
  static u32 deltasCount = 0;
  static const f32 fpsPrintFrequency = 1.0f; // Seconds

  deltasSum += _delta;
  deltasCount++;

  if (deltasSum >= fpsPrintFrequency)
  {
    double avg = deltasSum / deltasCount;
    IceLogInfo("} %4.3f ms -- %4.0f fps", avg * 1000, 1 / avg);

    // No need to update totals every frame
    totalDeltaSum += deltasSum;
    totalDeltaCount += deltasCount;

    deltasSum = 0;
    deltasCount = 0;
  }
  #else
  totalDeltaSum += _delta;
  totalDeltaCount++;
  #endif // ICE_DEBUG
  return true;
}

b8 Shutdown()
{
  f32 avg = totalDeltaSum / totalDeltaCount;
  // Force print
  Ice::ConsoleLogMessage(Ice::Log_Info,
                         "} Runtime: %4.2f -- Average delta: %4.3f ms -- %4.0f fps\n",
                         Ice::time.totalTime,
                         avg * 1000.0f,
                         1.0f / avg);
  return true;
}

int main()
{
  Ice::ApplicationSettings settings;
  settings.window.position = { 300, 150 };
  settings.window.extents = { 800, 600 };
  settings.window.title = "Test application";
  settings.renderer.api = Ice::Renderer_Vulkan;
  settings.clientInitFunction = Init;
  settings.clientUpdateFunction = Update;
  settings.clientShutdownFunction = Shutdown;

  return Ice::Run(settings);
}
