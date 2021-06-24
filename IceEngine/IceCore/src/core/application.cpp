
#include "defines.h"
#include "logger.h"
#include "core/application.h"
#include "platform/platform.h"
#include "platform/file_system.h"
#include "core/memory_manager.h"
#include "renderer/mesh.h"
#include "core/input.h"

void Application::Run()
{
  Initialize();
  MainLoop();
  Shutdown();
}

void Application::Initialize()
{
  IcePrint("ICE INIT =================================================");

  Platform.Initialize(800, 600, "Test ice");

  MemoryManager.Initialize();

  Renderer.Initialize();

  //ChildInit();

  // TODO : TEMPORARY -- Delete
  GetShaderProgramApp("test", Ice_Shader_Stage_Vert | Ice_Shader_Stage_Frag);
  //CreateObject();

  Renderer.RecordCommandBuffers();
}

void Application::MainLoop()
{
  IcePrint("ICE LOOP =================================================");
  while (Platform.Tick())
  {
    // Handle input
    
    // Run game code
    // Render
    Renderer.RenderFrame();
    Input.Update();
  }
}

void Application::Shutdown()
{
  IcePrint("ICE SHUTDOWN =============================================");
  Renderer.Shutdown();

  MemoryManager.Shutdown();
  Platform.Shutdown();
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

