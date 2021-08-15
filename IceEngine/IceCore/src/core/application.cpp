
#include "defines.h"
#include "logger.h"
#include "service_hub.h"

#include "core/application.h"
#include "core/event.h"
#include "core/input.h"
#include "core/memory_manager.h"
#include "platform/file_system.h"
#include "platform/platform.h"
#include "renderer/mesh.h"
#include "renderer/renderer.h"
#include "renderer/vulkan/vulkan_backend.h"

#include <glm/glm.hpp>

void IceApplication::Run()
{
  Initialize();
  MainLoop();
  Shutdown();
}

bool UpdateCamOnWindowResize(u16 _eventCode, void* _sender, void* _listener, IceEventData _data)
{
  IceApplication* app = static_cast<IceApplication*>(_listener);
  float x = (float)_data.u32[0];
  float y = (float)_data.u32[1];
  app->cam.SetProjection((x / y));
  return true;
}

void IceApplication::Initialize()
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
  EventManager.Register(Ice_Event_Window_Resized, this, UpdateCamOnWindowResize);

  ChildInit();
  ICE_ASSERT_MSG(ChildLoop != nullptr, "Failed to set an update function");

  #pragma region MoveToChildLoop
  renderer->CreateMesh("Cube.obj");
  u32 materialIndex = renderer->GetMaterial({"mvp", "blue"}, { Ice_Shader_Vert, Ice_Shader_Frag }, {});
  cam.position = glm::vec3(0, 0, 5);
  cam.SetRotation({ 0.0f, -90.0f, 0.0f });
  #pragma endregion

  renderer->RecordCommandBuffers(materialIndex);
}

void IceApplication::MainLoop()
{
  IceRenderPacket renderPacket {};

  LogInfo("ICE LOOP =================================================");
  while (platform->Update())
  {
    // Handle input

    // Run game code
    (this->*ChildLoop)();
    #pragma region MoveToChildLoop
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
    #pragma endregion

    // Render
    renderer->RenderFrame(&renderPacket);
    Input.Update();
  }
}

void IceApplication::Shutdown()
{
  LogInfo("ICE SHUTDOWN =============================================");
  ChildShutdown();
  vkDeviceWaitIdle(renderer->GetContext()->device);
  renderer->Shutdown();
  delete(renderer);

  MemoryManager.Shutdown();
  platform->Shutdown();
  delete(platform);
}

void IceApplication::CreateObject()
{
  //mesh_t m = renderer->CreateMesh("Cube.obj"); /*FileSystem::LoadMesh("Cube.obj");*/

}
