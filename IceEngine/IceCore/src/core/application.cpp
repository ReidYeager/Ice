
#include "defines.h"
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
  PlatformPrintToConsole("Ice App Init\n");

  platformState = new PlatformState();

  PlatformInitialize(platformState, 800, 600);
  IMemInitialize();

  a = IMemAllocate(12, IMEM_UNKNOWN);
  b = IMemAllocate(100, IMEM_BUFFER);
  c = IMemAllocate(540, IMEM_ARRAY);

  renderer = new Renderer();

  return 0;
}

i8 Application::MainLoop()
{
  PlatformPrintToConsole("Ice App MainLoop\n");

  while (!platformState->shouldClose)
  {
    PlatformPumpMessages();
    // Handle input
    // Run game code
    // Render
  }

  return 0;
}

i8 Application::Shutdown()
{
  PlatformPrintToConsole("Ice App Shutdown\n");

  delete(renderer);

  PlatformPrintToConsole("\n\n");
  IMemLogStats();
  IMemFree(a, 12, IMEM_UNKNOWN);
  IMemFree(b, 100, IMEM_BUFFER);
  IMemFree(c, 540, IMEM_ARRAY);
  PlatformPrintToConsole("\n\n");

  IMemShutdown();
  PlatformShutdown(platformState);
  return 0;
}
