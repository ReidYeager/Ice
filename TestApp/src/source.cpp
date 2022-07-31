
#include <stdio.h>
#include <ice.h>

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

  settings.window.extents = {800, 600};
  settings.window.position = {50, 50};
  settings.window.title = "Test title";

  Ice::Run(settings);
  return 0;
}

struct A { int x; int y; };
struct B { char c; };
struct C { f32 x; u64 f; };

b8 Init()
{
  //Ice::WindowSettings windowSettings {};
  //Ice::CreateWindow(windowSettings);

  Ice::Scene scene;
  Ice::Entity gun = scene.CreateEntity();
  Ice::Entity cube = scene.CreateEntity();
  Ice::Entity deleted = scene.CreateEntity();
  scene.DestroyEntity(deleted);
  Ice::Entity third = scene.CreateEntity();

  scene.AddComponent<A>(gun);
  scene.AddComponent<B>(cube);
  scene.AddComponent<A>(third);
  scene.AddComponent<B>(third);

  IceLogDebug("Gun   - (%u, %u)", gun.id, gun.version);
  IceLogDebug("Cube  - (%u, %u)", cube.id, cube.version);
  IceLogDebug("Third - (%u, %u)", third.id, third.version);

  IceLogInfo("");

  for (Ice::Entity a : Ice::SceneView<A>(scene))
  {
    IceLogInfo("%u, %u", a.id, a.version);
  }

  IceLogInfo("");

  for (Ice::Entity a : Ice::SceneView<B>(scene))
  {
    IceLogInfo("%u, %u", a.id, a.version);
  }

  IceLogInfo("");

  for (Ice::Entity a : Ice::SceneView<A, B>(scene))
  {
    IceLogInfo("%u, %u", a.id, a.version);
  }

  IceLogInfo("");

  return true;
}

b8 Update(f32 _delta)
{
  return false;
}

b8 Shutdown()
{
  return true;
}
