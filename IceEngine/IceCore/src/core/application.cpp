
#include "defines.h"
#include "logger.h"
#include "service_hub.h"
#include "core/application.h"
#include "core/memory_manager.h"
#include "core/input.h"
#include "core/event.h"
#include "platform/platform.h"
#include "platform/file_system.h"
#include "renderer/renderer.h"
#include "renderer/mesh.h"

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

#include "renderer/vulkan/vulkan_material.h"

void Application::Initialize()
{
  LogInfo("ICE INIT =================================================");

  platform = new IcePlatform();
  renderer = new IceRenderer();
  ServiceHub::Initialize(platform, renderer);

  EventManager.Initialize();

  platform->Initialize();
  platform->CreateWindow(800, 600, "Test Ice");
  //platform->ChangeCursorState(Ice_Cursor_Locked);

  MemoryManager.Initialize();

  renderer->Initialize(IceRenderer::Vulkan);
  cam.position = glm::vec3(0, 0, 5);
  cam.SetRotation({0.0f, -90.0f, 0.0f});

  EventManager.Register(Ice_Event_Window_Resized, this, UpdateCamOnWindowResize);

  //ChildInit();
  IvkMaterial testShader(renderer->GetContext(), {"blue"}, {Ice_Shader_Vert | Ice_Shader_Frag});

  renderer->CreateMesh("Cube.obj");
  u32 materialIndex = renderer->GetShaderProgram(renderer->GetContext(), "test", Ice_Shader_Vert | Ice_Shader_Frag,
                                       { "AltImage.png", "TestImage.png", "landscape.jpg"}, Ice_Pipeline_Cull_Mode_None);
  //CreateObject();

  renderer->RecordCommandBuffers();
}

void Application::MainLoop()
{
  IceRenderPacket renderPacket {};

  LogInfo("ICE LOOP =================================================");
  while (platform->Update())
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
      platform->Close();
    }

    // Render
    renderer->RenderFrame(&renderPacket);
    Input.Update();
  }
}

void Application::Shutdown()
{
  LogInfo("ICE SHUTDOWN =============================================");
  renderer->Shutdown();
  delete(renderer);

  MemoryManager.Shutdown();
  platform->Shutdown();
  delete(platform);
}

void Application::CreateObject()
{
  //mesh_t m = renderer->CreateMesh("Cube.obj"); /*FileSystem::LoadMesh("Cube.obj");*/

}
