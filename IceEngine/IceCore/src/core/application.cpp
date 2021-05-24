
#include "defines.h"
#include "core/application.h"
#include "platform/platform.h"
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

  return 0;
}

i8 Application::MainLoop()
{
  printf("Ice App MainLoop\n");

  //while (!m_shouldClose)
  //{
  //  
  //}
  return 0;
}

i8 Application::Shutdown()
{
  printf("Ice App Shutdown\n");

  Platform p;
  p.Initialize();
  p.Shutdown();

  return 0;
}
