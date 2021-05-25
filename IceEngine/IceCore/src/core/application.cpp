
#include "defines.h"
#include "core/application.h"
#include "platform/platform.h"
#include "core/memory_manager.h"
#include <iostream>

void Application::Run()
{
  Initialize();
  MainLoop();
  Shutdown();
}

i8 Application::Initialize()
{
  printf("Ice App Init\n");

  platformState = new PlatformState();

  PlatformInitialize(platformState, 800, 600);
  IMemInitialize();

  a = IMemAllocate(12, IMEM_UNKNOWN);
  b = IMemAllocate(100, IMEM_BUFFER);
  c = IMemAllocate(540, IMEM_ARRAY);

  return 0;
}

i8 Application::MainLoop()
{
  printf("Ice App MainLoop\n");

  //while (!m_shouldClose)
  //{
  //  PlatformPumpMessages();
  //}

  return 0;
}

i8 Application::Shutdown()
{
  printf("Ice App Shutdown\n");

  IMemLogStats();

  IMemFree(a, 12, IMEM_UNKNOWN);
  IMemFree(b, 100, IMEM_BUFFER);
  IMemFree(c, 540, IMEM_ARRAY);

  printf("\n\n");
  IMemLogStats();

  PlatformShutdown(platformState);
  return 0;
}
