
#include "defines.h"
#include "logger.h"

#include "core/application.h"
#include "core/input.h"
#include "core/object.h"
#include "libraries/imgui/imgui.h"
#include "libraries/imgui/imgui_impl_win32.h"
#include "libraries/imgui/imgui_impl_vulkan.h"

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <chrono>

void AddObjectToSceneGui(IceObject* _object, u32 index = 0)
{
  if (ImGui::TreeNode((void*)index, "object %d", index))
  {
    ImGui::DragFloat3("Position", &_object->transform.position[0], 0.01f);
    ImGui::DragFloat3("Rotation", &_object->transform.rotation[0], 0.1f);
    ImGui::DragFloat3("Scale", &_object->transform.scale[0], 0.01f);

    _object->transform.UpdateMatrix();

    for (const auto& o : _object->children)
    {
      AddObjectToSceneGui(o, ++index);
    }

    ImGui::TreePop(); // Done displaying this object's information
  }
}

u32 reIceApplication::Run(IceApplicationSettings* _settings)
{
  try
  {
    if (!Initialize(_settings))
    {
      IceLogFatal("IceApplication Initialization failed");
      return -1;
    }

    if (!Update())
    {
      IceLogFatal("IceApplication Update failed");
      return -2;
    }

    if (!Shutdown())
    {
      IceLogFatal("IceApplication Shutdown failed");
      return -3;
    }

    return 0;
  }
  catch (const char* error)
  {
    IceLogFatal("Ice caught :: %s", error);
    return -4;
  }
}

b8 reIceApplication::Initialize(IceApplicationSettings* _settings)
{
  state.ClientInitialize = _settings->ClientInitialize;
  state.ClientUpdate = _settings->ClientUpdate;
  state.ClientShutdown = _settings->ClientShutdown;

  IceLogInfo("===== reApplication Initialize =====");

  // Initialize the platform =====
  _settings->windowSettings.title = _settings->title;
  if (!rePlatform.Initialize(&_settings->windowSettings))
  {
    IceLogFatal("Ice Platform failed to initialize");
    return false;
  }
  Input.Initialize();

  // Initialize the renderer =====
  if (!renderer.Initialize(_settings->rendererSettings))
  {
    IceLogFatal("Ice Renderer failed to initialize");
    return false;
  }

  sceneRoot = new IceObject();
  sceneRoot->transform.matrix = glm::mat4(1.0f);

  // Set camera default state =====
  glm::mat4 viewProj = glm::mat4(1);
  viewProj = glm::perspective(glm::radians(90.0f), 1280.0f / 720.0f, 0.01f, 1000.0f);
  viewProj[1][1] *= -1; // Account for Vulkan's inverted Y screen coord
  viewProj = glm::translate(viewProj, glm::vec3(0.0f, 0.0f, -3.0f));
  cam.viewProjectionMatrix = viewProj;

  // Initialize the client game =====
  state.ClientInitialize();

  return true;
}

b8 reIceApplication::Update()
{
  IceLogInfo("===== reApplication Main Loop =====");

  auto start = std::chrono::steady_clock::now();
  auto end = start;
  auto microsecDelta = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  float deltaTime = 0.0f;
  const float microToMilli = 0.001f;
  const float microToSecond = microToMilli * 0.001f;

  float deltaSum = 0.0f;
  u32 deltaCount = 0;

  float totalAverageDeltaSum = 0.0f;
  u32 totalAverageDeltaCount = 0;
 
  while (rePlatform.Update())
  {
    // Start IMGUI frame
    {
      ImGui_ImplWin32_NewFrame();
      ImGui_ImplVulkan_NewFrame();
      ImGui::NewFrame();

      ImGui::ShowDemoWindow();
    }

    // Scene hierarchy
    {
      u32 index = 0;
      ImGui::Begin("Scene");
      for (auto& o : sceneRoot->children)
      {
        AddObjectToSceneGui(o, index++);
      }
      //AddObjectToSceneGui(sceneRoot, 0);
      ImGui::End();
    }

    state.ClientUpdate(deltaTime);

    ICE_ATTEMPT_BOOL(renderer.Render(&cam));

    // Update timing =====
    {
      end = std::chrono::steady_clock::now();
      microsecDelta = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
      deltaTime = microsecDelta.count() * microToSecond;
      start = end;

      totalTime += deltaTime;

      // Timing log =====
      deltaSum += deltaTime;
      deltaCount++;
      if (deltaSum >= 1.0f)
      {
        float averageSec = (deltaSum / deltaCount);
        IceLogInfo("%5.3f ms -- %3.0f FPS", averageSec * 1000.0f, 1.0f / averageSec);

        totalAverageDeltaSum += averageSec;
        totalAverageDeltaCount++;

        deltaSum = 0;
        deltaCount = 0;
      }

      // Update input =====
      Input.Update();
    }
  }

  float totalAverageDelta = totalAverageDeltaSum / totalAverageDeltaCount;
  IceLogInfo("Average delta : %3.3f ms -- %3.0f FPS",
             totalAverageDelta * 1000.0f,
             1.0f / totalAverageDelta);

  return true;
}

void DestroyObjectAndChildren(IceObject* object)
{
  for (auto& o : object->children)
  {
    DestroyObjectAndChildren(o);
  }

  object->children.clear();
  free(object);
}

b8 reIceApplication::Shutdown()
{
  IceLogInfo("===== reApplication Shutdown =====");
  state.ClientShutdown();

  DestroyObjectAndChildren(sceneRoot);

  renderer.Shutdown();
  rePlatform.Shutdown();
  return true;
}

IceObject* reIceApplication::AddObject(const char* _meshDir, u32 _material, IceObject* _parent)
{
  IceObject* obj = new IceObject();
  obj->materialHandle = _material;
  obj->meshHandle = renderer.CreateMesh(_meshDir);

  if (_parent != nullptr)
  {
    obj->transform.parent = &_parent->transform;
    _parent->children.push_back(obj);
  }
  else
  {
    obj->transform.parent = &sceneRoot->transform;
    sceneRoot->children.push_back(obj);
  }

  renderer.AddObjectToScene(obj);
  obj->transform.UpdateMatrix();

  return obj;
}

IceHandle reIceApplication::CreateMaterial(IceMaterialTypes _type, std::vector<IceShader> _shaders)
{
  IceHandle mat = renderer.CreateMaterial(_shaders, _type, 0);

  if (mat == ICE_NULL_HANDLE)
  {
    throw "Failed to create a material";
  }

  return mat;
}

u32 reIceApplication::CreateLightingMaterial(std::vector<IceShader> _shaders)
{
  return renderer.CreateMaterial(_shaders, Ice_Mat_Deferred_Light, 1);
}

b8 reIceApplication::SetLightingMaterial(IceHandle _material)
{
  return renderer.SetLightingMaterial(_material);
}

void reIceApplication::AssignMaterialTextures(IceHandle _material,
                                              std::vector<IceTexture> _textures)
{
  renderer.AssignMaterialTextures(_material, _textures);
}

b8 reIceApplication::SetMaterialBufferData(IceHandle _material, void* _data)
{
  return renderer.SetMaterialBufferData(_material, _data);
}

glm::mat4 IceTransform::UpdateMatrix()
{
  matrix = parent->matrix;

  matrix = glm::translate(matrix, position);
  matrix = glm::rotate(matrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
  matrix = glm::rotate(matrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
  matrix = glm::rotate(matrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
  matrix = glm::scale(matrix, scale);

  renderer.FillBuffer(&buffer, &matrix, 64);

  // TODO : Update children's matrices

  return matrix;
}
