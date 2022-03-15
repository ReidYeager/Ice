
#include <ice.h>

b8 Init()
{
  // TODO : ~!!~ Materials
  //Ice::MaterialSettings lightMatSettings;
  //lightMatSettings.vertFile = "_light_blank";
  //lightMatSettings.fragFile = "_light_blank";
  //lightMatSettings.subpass = 1;

  //Ice::Material lightMat = Ice::CreateMaterial(lightMatSettings);
  //Ice::SetLightingMaterial(lightMat);

  //Ice::MaterialSettings blankMatSettings { "blank_deferred", "blank_deferred" };
  //Ice::Material blank = Ice::CreateMaterial(blankMatSettings);

  return true;
}

b8 Update()
{
  if (Input.OnKeyPressed(Ice_Key_Escape))
  {
    Ice::platform.Shutdown();
  }

  return true;
}

b8 Shutdown()
{
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
