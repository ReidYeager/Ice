
#include <ice.h>

#ifdef ICE_DEBUG
// Used to print the average frame time for this session
f32 totalDeltaSum = 0.0;
u32 totalDeltaCount = 0;
#endif // ICE_DEBUG

b8 Init()
{
  Ice::MaterialSettings materialSettings;
  Ice::Shader shaderInfo;
  shaderInfo.fileDirectory = "_light_blank";
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

  materialSettings.shaders[0].fileDirectory = "_light_half";
  materialSettings.shaders[1].fileDirectory = "_light_half";
  Ice::CreateMaterial(materialSettings);

  return true;
}

b8 Update(f32 _delta)
{
  if (Input.OnKeyPressed(Ice_Key_Escape))
  {
    Ice::Shutdown();
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
  #endif // ICE_DEBUG
  return true;
}

b8 Shutdown()
{
  #ifdef ICE_DEBUG
  f32 avg = totalDeltaSum / totalDeltaCount;
  IceLogInfo("} Average frame time: %4.3f ms -- %4.0f fps", avg * 1000, 1 / avg);
  #endif // ICE_DEBUG
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
