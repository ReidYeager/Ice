
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
  GetShaderProgramApp("test", Ice_Shader_Stage_Vert | Ice_Shader_Stage_Frag);
  //CreateObject();

  renderer->RecordCommandBuffers();
}

void Application::MainLoop()
{
  IcePrint("ICE LOOP =================================================");
  while (platform->Tick())
  {
    renderer->RenderFrame();
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
  //mesh_t m = renderer->CreateMesh("Cube.obj"); /*FileSystem::LoadMesh("Cube.obj");*/

}

void Application::GetShaderProgramApp(const char* _name, IceShaderStageFlags _stages)
{
  //renderer->GetShaderProgram(_name, _stages);
  GetShaderProgram(_name, _stages);
}

