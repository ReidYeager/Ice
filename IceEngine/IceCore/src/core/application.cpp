
#include "defines.h"
#include "logger.h"
#include "core/application.h"
#include "platform/platform.h"
#include "platform/file_system.h"
#include "core/memory_manager.h"
#include "renderer/mesh.h"
#include "core/input.h"
#include "core/event.h"

#include <glm/glm.hpp>

void Application::Run()
{
  Initialize();
  MainLoop();
  Shutdown();
}

bool UpdateCamOnWindowResize(u16 _eventCode, void* _sender, void* _listener, IceEventData _data)
{
  Application* app = static_cast<Application*>(_listener);
  float x = (float)_data.u32[0];
  float y = (float)_data.u32[1];
  app->cam.SetProjection((x / y));
  return true;
}

void Application::Initialize()
{
  IcePrint("ICE INIT =================================================");

  EventManager.Initialize();

  Platform.Initialize(800, 600, "Test ice");
  Platform.ChangeCursorState(Ice_Cursor_Locked);

  MemoryManager.Initialize();

  Renderer.Initialize();
  cam.position = glm::vec3(0, 0, 5);
  cam.SetRotation({0.0f, -90.0f, 0.0f});

  EventManager.Register(Ice_Event_Window_Resized, this, UpdateCamOnWindowResize);

  //ChildInit();

  Renderer.CreateMesh("Sphere.obj");
  u32 materialIndex = Renderer.GetShaderProgram("test", Ice_Shader_Stage_Vert | Ice_Shader_Stage_Frag,
                                       { "AltImage.png", "TestImage.png", "landscape.jpg"}, Ice_Pipeline_Cull_Mode_None);
  //CreateObject();

  Renderer.RecordCommandBuffers();
}

void Application::MainLoop()
{
  IceRenderPacket renderPacket {};

  IcePrint("ICE LOOP =================================================");
  while (Platform.Tick())
  {
    // Handle input

    // Run game code
    //i32 x, y;
    //Input.GetMouseDelta(&x, &y);
    //cam.Rotate({-y, x, 0});
    //cam.ClampPitch(89.0f, -89.0f);

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
    renderPacket.projectionMatrix = cam.GetProjectionMatrix();

    if (Input.IsKeyDown(Ice_Key_Escape))
    {
      Platform.Close();
    }

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
