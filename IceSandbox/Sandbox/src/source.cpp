
#include <ice.h>

// Used to print the average frame time for this session
f32 totalDeltaSum = 0.0;
u32 totalDeltaCount = 0;

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

  Ice::Entity entityA = CreateObject("Cyborg_Weapon.obj", lightMat);
  Ice::Entity entityB = CreateObject("Sphere.obj", lightMat);

  entityA.transform.position.y = 0.3f;
  entityB.transform.position.z = 1.0f;

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
  settings.window.position = { 50, 50 };
  settings.window.extents = { 800, 600 };
  settings.window.title = "Test application";
  settings.renderer.api = Ice::Renderer_Vulkan;
  settings.clientInitFunction = Init;
  settings.clientUpdateFunction = Update;
  settings.clientShutdownFunction = Shutdown;

  return Ice::Run(settings);
}
