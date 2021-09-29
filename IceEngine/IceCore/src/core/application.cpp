
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
#include <chrono>
#include <iostream>

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
  ecsController = new IceEcsController();
  ServiceHub::Initialize(platform, renderer);

  EventManager.Initialize();

  platform->Initialize();
  platform->CreateWindow(800, 600, "Test Ice");
  //platform->ChangeCursorState(Ice_Cursor_Locked);

  MemoryManager.Initialize();

  renderer->Initialize(IceRenderer::Vulkan);
  EventManager.Register(Ice_Event_Window_Resized, this, UpdateCamOnWindowResize);

  ecsController->registry.on_construct<RenderableComponent>().connect<&IceApplication::RenderableCallback>(this);

  ChildInit();
  ICE_ASSERT_MSG(ChildLoop != nullptr, "Failed to set an update function");

  #pragma region MoveToChildLoop
  cam.position = glm::vec3(0.0f, 0.0f, 5.0f);
  cam.SetRotation({ 0.0f, -90.0f, 0.0f });
  #pragma endregion
}

void IceApplication::MainLoop()
{
  auto start = std::chrono::steady_clock::now();
  auto end = std::chrono::steady_clock::now();
  auto deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  //i32 x, y;
  const float camMoveSpeed = 3.0f;
  //const float sensitivity = 0.2f;

  float deltasSum = 0.0f;
  int deltasCount = 0;
  float fpsPrintFrequency = 1.0f;  // Seconds
  fpsPrintFrequency *= 1000000.0f; // To microseconds

  LogInfo("ICE LOOP =================================================");
  while (platform->Update())
  {
    start = end;
    renderPacket.deltaTime = deltaTime.count() * 0.000001f;

    // Handle input

    // Run game code
    (this->*ChildLoop)();
    #pragma region MoveToChildLoop
    //int x, y;
    //Input.GetMouseDelta(&x, &y);
    //cam.Rotate({-y * 0.1f, x * 0.1f, 0});
    //cam.ClampPitch(89.0f, -89.0f);

    if (Input.IsKeyDown(Ice_Key_W))
      cam.position += cam.GetForward() * renderPacket.deltaTime * camMoveSpeed;
    if (Input.IsKeyDown(Ice_Key_S))
      cam.position -= cam.GetForward() * renderPacket.deltaTime * camMoveSpeed;
    if (Input.IsKeyDown(Ice_Key_D))
      cam.position += cam.GetRight() * renderPacket.deltaTime * camMoveSpeed;
    if (Input.IsKeyDown(Ice_Key_A))
      cam.position -= cam.GetRight() * renderPacket.deltaTime * camMoveSpeed;
    if (Input.IsKeyDown(Ice_Key_E))
      cam.position += glm::vec3(0.0f, renderPacket.deltaTime, 0.0f) * camMoveSpeed;
    if (Input.IsKeyDown(Ice_Key_Q))
      cam.position -= glm::vec3(0.0f, renderPacket.deltaTime, 0.0f) * camMoveSpeed;

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

    end = std::chrono::steady_clock::now();
    deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    deltasSum += deltaTime.count();
    deltasCount++;

    if (deltasSum >= fpsPrintFrequency)
    {
      float avg = deltasSum / deltasCount;
      LogInfo(" %8.0f us -- %4.0f fps", avg, 1.0f / (avg * 0.000001f));

      deltasSum = 0;
      deltasCount = 0;
    }

  }
}

void IceApplication::Shutdown()
{
  LogInfo("ICE SHUTDOWN =============================================");
  ChildShutdown();

  ecsController->registry.on_construct<RenderableComponent>().disconnect<&IceApplication::RenderableCallback>(this);

  vkDeviceWaitIdle(renderer->GetContext()->device);
  renderer->Shutdown();
  delete(renderer);

  MemoryManager.Shutdown();
  platform->Shutdown();
  delete(platform);
}

void IceApplication::RenderableCallback()
{
  auto v = ecsController->registry.view<RenderableComponent>();
  for (auto e : v)
  {
    RenderableComponent rc = ecsController->GetComponent<RenderableComponent>(e);
    renderPacket.renderables.push_back(&(renderer->meshes[rc.meshIndex]));
    renderPacket.materialIndices.push_back(rc.materialIndex);
  }
}

GameObject IceApplication::CreateObject()
{
  GameObject g(ecsController);

  return g;
}

u32 IceApplication::GetMeshIndex(const char* _meshDir /*= nullptr*/)
{
  if (_meshDir != nullptr)
    return renderer->CreateMesh(_meshDir);

  return -1;
}

u32 IceApplication::GetMaterialIndex(std::vector<const char*> _shaderNames,
                                     std::vector<IceShaderStageFlags> _shaderStages,
                                     std::vector<const char*> _texStrings,
                                     IceFlag _renderSettings /*= 0*/)
{
  return renderer->GetMaterial(_shaderNames, _shaderStages, _texStrings, _renderSettings);
}
