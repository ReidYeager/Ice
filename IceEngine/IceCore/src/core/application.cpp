
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
  IceRenderPacket renderPacket {};
  renderPacket.projectionMatrix = cam.UpdateProjection((800.0f/600.0f));

  IcePrint("ICE LOOP =================================================");
  while (Platform.Tick())
  {
    // Handle input
    
    // Run game code
    i32 x, y;
    Input.GetMouseDelta(&x, &y);
    cam.Rotate({-y, x, 0});
    cam.ClampPitch(89.0f, -89.0f);

    if (Input.IsKeyDown(Ice_Key_W))
      cam.position += cam.GetForward() * 0.002f;
    if (Input.IsKeyDown(Ice_Key_S))
      cam.position -= cam.GetForward() * 0.002f;
    if (Input.IsKeyDown(Ice_Key_D))
      cam.position += cam.GetRight() * 0.002f;
    if (Input.IsKeyDown(Ice_Key_A))
      cam.position -= cam.GetRight() * 0.002f;
    if (Input.IsKeyDown(Ice_Key_E))
      cam.position += glm::vec3(0.0f, 0.002f, 0.0f);
    if (Input.IsKeyDown(Ice_Key_Q))
      cam.position -= glm::vec3(0.0f, 0.002f, 0.0f);

    renderPacket.viewMatrix = cam.GetViewMatrix();

    // Render
    Renderer.RenderFrame(&renderPacket);
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
  GetShaderProgram(_name, _stages, Ice_Pipeline_Cull_Mode_Back);
}

