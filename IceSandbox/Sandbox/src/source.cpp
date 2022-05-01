
#include <ice.h>

// Used to print the average frame time for this session
f32 totalDeltaSum = 0.0;
u32 totalDeltaCount = 0;

Ice::Object* gun;
Ice::Object* sphere;
Ice::Object* camera;
Ice::Transform* eTransform;
b8 transformingCamera = false;

Ice::Material* materialA;
Ice::Material* materialB;

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

  //Ice::SetLightingMaterial(lightMat);
  Ice::CreateMaterial(materialSettings, &materialA);
  Ice::CreateMaterial(materialSettings, &materialB);

  gun = &Ice::CreateObject();
  Ice::AttatchRenderComponent(gun, "Cyborg_Weapon.obj", materialA);
  gun->transform->SetRotation({0.0f, 90.0f, 0.0f});

  //sphere = &Ice::CreateObject();
  //Ice::AttatchRenderComponent(sphere, "SphereSmooth.obj", materialB);

  //sphere->transform->SetPosition({0.0f, -1.0f, 0.0f});

  Ice::CameraSettings camSettings;
  camSettings.isProjection = true;
  camSettings.horizontal = 45.0f;
  camSettings.ratio = 800.0f / 600.0f;
  camSettings.nearClip = 0.01f;
  camSettings.farClip = 10.0f;

  camera = &Ice::CreateObject();
  Ice::AttatchCameraComponent(camera, camSettings);
  camera->transform->SetPosition({0.0f, 0.0f, 3.0f});

  eTransform = gun->transform;

  //sphere->transform->SetParent(eTransform);

  Ice::SetTexture(materialA, 0, "CyborgWeapon/Weapon_ao.png");

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

  if (Input.OnKeyPressed(Ice_Key_C))
  {
    eTransform = transformingCamera ? gun->transform : camera->transform;
    transformingCamera = !transformingCamera;
    IceLogDebug("Now controlling the %s", transformingCamera? "camera" : "gun");
  }

  //static b8 isAo = true;
  //if (Input.OnKeyPressed(Ice_Key_M))
  //{
  //  Ice::SetTexture(materialA, 0, isAo? "CyborgWeapon/Weapon_albedo.png" : "CyborgWeapon/Weapon_ao.png");
  //  isAo = !isAo;
  //}

  const f32 degreesPerSecond = 90.0f;
  vec3 transformVec;
  static vec3 transformRot = {0.0f, 0.0f, 0.0f};
  transformVec = {0.0f, 0.0f, 0.0f};

  transformVec.x += _delta * Input.IsKeyDown(Ice_Key_D);
  transformVec.x -= _delta * Input.IsKeyDown(Ice_Key_A);

  transformVec.y += _delta * Input.IsKeyDown(Ice_Key_E);
  transformVec.y -= _delta * Input.IsKeyDown(Ice_Key_Q);

  transformVec.z += _delta * Input.IsKeyDown(Ice_Key_S);
  transformVec.z -= _delta * Input.IsKeyDown(Ice_Key_W);

  eTransform->Translate(transformVec);

  transformVec = { 0.0f, 0.0f, 0.0f };
  transformVec.x += degreesPerSecond * _delta * Input.IsKeyDown(Ice_Key_I);
  transformVec.x -= degreesPerSecond * _delta * Input.IsKeyDown(Ice_Key_K);

  transformVec.y += degreesPerSecond * _delta * Input.IsKeyDown(Ice_Key_J);
  transformVec.y -= degreesPerSecond * _delta * Input.IsKeyDown(Ice_Key_L);

  transformVec.z += degreesPerSecond * _delta * Input.IsKeyDown(Ice_Key_U);
  transformVec.z -= degreesPerSecond * _delta * Input.IsKeyDown(Ice_Key_O);

  eTransform->Rotate(transformVec);

  //transformRot.x += degreesPerSecond * _delta * Input.IsKeyDown(Ice_Key_I);
  //transformRot.x -= degreesPerSecond * _delta * Input.IsKeyDown(Ice_Key_K);

  //transformRot.y += degreesPerSecond * _delta * Input.IsKeyDown(Ice_Key_J);
  //transformRot.y -= degreesPerSecond * _delta * Input.IsKeyDown(Ice_Key_L);

  //transformRot.z += degreesPerSecond * _delta * Input.IsKeyDown(Ice_Key_U);
  //transformRot.z -= degreesPerSecond * _delta * Input.IsKeyDown(Ice_Key_O);

  //eTransform->SetRotation(transformRot);

  transformVec = { 0.0f, 0.0f, 0.0f };
  transformVec.x += _delta * Input.IsKeyDown(Ice_Key_H);
  transformVec.x -= _delta * Input.IsKeyDown(Ice_Key_F);

  transformVec.y += _delta * Input.IsKeyDown(Ice_Key_T);
  transformVec.y -= _delta * Input.IsKeyDown(Ice_Key_G);

  transformVec.z += _delta * Input.IsKeyDown(Ice_Key_Y);
  transformVec.z -= _delta * Input.IsKeyDown(Ice_Key_R);

  eTransform->Scale(transformVec);

  //camera->transform->position.x = cos(Ice::time.totalTime) * 2.5f;
  //camera->transform->position.z = sin(Ice::time.totalTime) * 2.5f;
  //camera->transform->rotation.y = (-Ice::time.totalTime * 57.2958279088f) + 90.0f;

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
