
#include "defines.h"
#include "logger.h"
#include "core/application.h"
#include "platform/platform.h"
#include "platform/file_system.h"
#include "core/memory_manager.h"
#include "renderer/mesh.h"

void Application::Run()
{
  Initialize();
  MainLoop();
  Shutdown();
}

void Application::Initialize()
{
  IcePrint("ICE INIT =================================================");

  platform = new Platform(800, 600, "Test ice");

  MemoryManager::Initialize();

  renderer = new Renderer();

  //ChildInit();

  // TODO : TEMPORARY -- Delete
  GetShaderProgram("default", ICE_SHADER_STAGE_VERT | ICE_SHADER_STAGE_FRAG);
  CreateObject();

}

void Application::MainLoop()
{
  IcePrint("ICE LOOP =================================================");
  while (platform->Tick())
  {
    // Handle input
    // Run game code
    // Render
  }
}

void Application::Shutdown()
{
  IcePrint("ICE SHUTDOWN =============================================");
  delete(renderer);

  MemoryManager::Shutdown();
  delete(platform);
}

void Application::CreateObject()
{
  mesh_t m = FileSystem::LoadMesh("Cube.obj");
}

void Application::GetShaderProgram(const char* _name, IceShaderStageFlags _stages)
{
  renderer->GetShaderProgram(_name, _stages);
}

