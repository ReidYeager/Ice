
#include "defines.h"
#include "logger.h"
#include "core/application.h"
#include "platform/platform.h"
#include "core/memory_manager.h"

void Application::Run()
{
  Initialize();
  MainLoop();
  Shutdown();
}

i8 Application::Initialize()
{
  IcePrint("ICE INIT =================================================");

  platform = new Platform(800, 600, "Test ice");

  MemoryManager::Initialize();
  a = MemoryManager::Allocate(12, IMEM_TYPE_UNKNOWN);
  b = MemoryManager::Allocate(100, IMEM_TYPE_BUFFER);
  c = MemoryManager::Allocate(540, IMEM_TYPE_BUFFER);
  MemoryManager::PrintStats();

  renderer = new Renderer();

  return 0;
}

i8 Application::MainLoop()
{
  IcePrint("ICE LOOP =================================================");
  while (platform->Tick())
  {
    // Handle input
    // Run game code
    // Render
  }

  return 0;
}

i8 Application::Shutdown()
{
  IcePrint("ICE SHUTDOWN =============================================");
  delete(renderer);

  MemoryManager::Free(a, 12, IMEM_TYPE_UNKNOWN);
  MemoryManager::Free(b, 100, IMEM_TYPE_BUFFER);
  MemoryManager::Free(c, 540, IMEM_TYPE_BUFFER);

  MemoryManager::Shutdown();
  delete(platform);
  return 0;
}
