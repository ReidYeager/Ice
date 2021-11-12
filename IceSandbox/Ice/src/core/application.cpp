
#include "defines.h"
#include "asserts.h"
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

#include <chrono>
#include <iostream>
#include <glm/glm.hpp>

void IceApplication::Initialize(void(*_gameInit)(), void(*_gameLoop)(float), void(*_gameShutdown)())
{
  gameInit = _gameInit;
  gameLoop = _gameLoop;
  gameShutdown = _gameShutdown;

  ICE_ASSERT(gameInit != 0 && gameLoop != 0 && gameShutdown != 0);

  Input = Input;
}

void IceApplication::Run()
{
  try
  {
    Startup();
    MainLoop();
    Shutdown();
  }
  catch (const char* e)
  {
    IceLogFatal("Ice caught : %s", e);
  }
}

void IceApplication::Close()
{
  platform->Close();
}

// Registered to the window resize event
bool UpdateCamOnWindowResize(u16 _eventCode, void* _sender, void* _listener, IceEventData _data)
{
  IceApplication* app = static_cast<IceApplication*>(_listener);
  float x = (float)_data.u32[0];
  float y = (float)_data.u32[1];
  app->cam.SetProjection(x / y);
  return true;
}

void IceApplication::Startup()
{
  IceLogInfo("ICE INIT =================================================");

  // Create components
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

  // Register callbacks
  EventManager.Register(Ice_Event_Window_Resized, this, UpdateCamOnWindowResize);
  ecsController->registry.
      on_construct<RenderableComponent>().connect<&IceApplication::RenderableCallback>(this);

  // Initialize user's application
  gameInit();

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

  float deltasSum = 0.0f;
  int deltasCount = 0;
  const float fpsPrintFrequency = 1000000.0f;  // Microseconds

  const float microToSeconds = 0.000001f;

  // Update objects to be rendered
  // NOTE : Should find a more efficient way of accomplishing this goal
  renderPacket.transforms.clear();
  auto v = ecsController->registry.view<RenderableComponent>(); // Guaranteed to have a transform
  for (auto e : v)
  {
    renderPacket.transforms.push_back(&ecsController->GetComponent<TransformComponent>(e));
  }

  IceLogInfo("ICE LOOP =================================================");
  while (platform->Update())
  {
    // Handle input
    Input.Update();

    // Run game code
    gameLoop(renderPacket.deltaTime);

    renderPacket.viewMatrix = cam.GetViewMatrix();
    renderPacket.projectionMatrix = cam.GetProjectionMatrix();

    // Update objects to be rendered

    // Render
    renderer->RenderFrame(&renderPacket);

    // Calculate times
    {
      end = std::chrono::steady_clock::now();
      deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

      deltasSum += deltaTime.count();
      deltasCount++;

      if (deltasSum >= fpsPrintFrequency)
      {
        float avg = deltasSum / deltasCount;
        IceLogInfo(" %8.0f us -- %4.0f fps", avg, 1.0f / (avg * microToSeconds));

        deltasSum = 0;
        deltasCount = 0;
      }

      start = end;
      renderPacket.deltaTime = deltaTime.count() * microToSeconds;
    }

  }
}

void IceApplication::Shutdown()
{
  IceLogInfo("ICE SHUTDOWN =============================================");
  gameShutdown();

  EventManager.Unregister(Ice_Event_Window_Resized, this, UpdateCamOnWindowResize);
  ecsController->registry.
      on_construct<RenderableComponent>().disconnect<&IceApplication::RenderableCallback>(this);

  // Shutdown components
  vkDeviceWaitIdle(renderer->GetContext()->device);
  renderer->Shutdown();
  delete(renderer);

  MemoryManager.Shutdown();
  platform->Shutdown();
  delete(platform);
}

void IceApplication::RenderableCallback()
{
  // Refresh the list of objects to render
  // NOTE : Should find a more efficient way of accomplishing this goal
  renderPacket.renderables.clear();
  renderPacket.materialIndices.clear();
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

void IceApplication::MaterialUpdateBuffer(u32 _material,
                                           IceShaderStageFlags _stage,
                                           IceShaderBufferParameterFlags _userParameterFlags,
                                           void* _userData)
{
  renderer->UpdateMaterialBuffer(_material, _stage, _userParameterFlags, _userData);
}

void IceApplication::MaterialUpdateTextures(u32 _material, std::vector<const char*> _textureNames)
{
  //renderer->UpdateMaterialImages(_material, _textureNames);
}
