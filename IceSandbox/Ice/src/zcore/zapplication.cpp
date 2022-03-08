
#include "defines.h"
#include "logger.h"

#include "zcore/zapplication.h"
#include "core/input.h"

#include "libraries/imgui/imgui.h"
#include "libraries/imgui/imgui_impl_win32.h"
#include "libraries/imgui/imgui_impl_vulkan.h"
#include "core/object.h"

#include <chrono>

Ice::IceTime Ice::time;
b8(*gameUpdate)() = 0;
b8(*gameShutdown)() = 0;
zIceWindow window;

IceCamera cam;
IceObject* sceneRoot;
void AddObjectToSceneGuiAlt(IceObject* _object, u32 index = 0)
{
  if (ImGui::TreeNode((void*)index, "object %d", index))
  {
    ImGui::DragFloat3("Position", &_object->transform.position[0], 0.01f);
    ImGui::DragFloat3("Rotation", &_object->transform.rotation[0], 0.1f);
    ImGui::DragFloat3("Scale", &_object->transform.scale[0], 0.01f);

    _object->transform.UpdateMatrix();

    for (const auto& o : _object->children)
    {
      AddObjectToSceneGuiAlt(o, ++index);
    }

    ImGui::TreePop(); // Done displaying this object's information
  }
}
void DestroyObjectAndChildrenAlt(IceObject* object)
{
  for (auto& o : object->children)
  {
    DestroyObjectAndChildrenAlt(o);
  }

  object->children.clear();
  free(object);
}

b8 Init(Ice::zIceApplicationSettings _settings)
{
  Input.Initialize();

  window = Ice::CreateNewWindow(_settings.windowSettings);

  _settings.rendererSettings.window = &window;

  ICE_ATTEMPT_BOOL(renderer.Initialize(_settings.rendererSettings));

  sceneRoot = new IceObject();
  sceneRoot->transform.matrix = glm::mat4(1.0f);

  ICE_ATTEMPT_BOOL(_settings.gameInit());
  gameUpdate = _settings.gameUpdate;
  gameShutdown = _settings.gameShutdown;

  Ice::InitTime();

  {
    // Set camera default state =====
    glm::mat4 viewProj = glm::mat4(1);
    vec2U extents = _settings.windowSettings.extents;
    viewProj = glm::perspective(glm::radians(90.0f), float(extents.x) / float(extents.y), 0.01f, 1000.0f);
    viewProj[1][1] *= -1; // Account for Vulkan's inverted Y screen coord
    viewProj = glm::translate(viewProj, glm::vec3(0.0f, 0.0f, -3.0f));
    cam.viewProjectionMatrix = viewProj;
  }

  return true;
}

b8 Update()
{
  float avgDelta = 0.0f;
  float deltaSum = 0.0f;
  u32 frameCount = 0;

  while (window.Update())
  {
    // Start IMGUI frame
    //{
    //  ImGui_ImplWin32_NewFrame();
    //  ImGui_ImplVulkan_NewFrame();
    //  ImGui::NewFrame();
    //}
    //// Scene hierarchy
    //{
    //  u32 index = 0;
    //  ImGui::Begin("Scene");
    //  for (auto& o : sceneRoot->children)
    //  {
    //    AddObjectToSceneGuiAlt(o, index++);
    //  }
    //  //AddObjectToSceneGui(sceneRoot, 0);
    //  ImGui::End();
    //}

    ICE_ATTEMPT_BOOL(gameUpdate());
    ICE_ATTEMPT_BOOL(renderer.Render(&cam));

    Input.Update();
    Ice::UpdateTime();

    // Debug timing log =====
    deltaSum += Ice::time.deltaTime;
    frameCount++;
    if (deltaSum >= 1.0f)
    {
      avgDelta = deltaSum / frameCount;
      IceLogInfo("%u frames : %5.3f ms : %f FPS",
                  frameCount,
                  avgDelta * 1000.0f,
                  1.0f / avgDelta);

      deltaSum = 0.0f;
      frameCount = 0;
    }
  }

  return true;
}

b8 Shutdown()
{
  ICE_ATTEMPT_BOOL(gameShutdown());

  window.Close();

  DestroyObjectAndChildrenAlt(sceneRoot);

  return true;
}

b8 Ice::Run(zIceApplicationSettings _settings)
{
  try
  {
    if (!Init(_settings))
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

IceObject* Ice::zAddObject(const char* _meshDir, u32 _material, IceObject* _parent)
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

IceHandle Ice::zCreateMaterial(IceMaterialTypes _type, std::vector<IceShader> _shaders)
{
  IceHandle mat = renderer.CreateMaterial(_shaders, _type, 0);

  if (mat == ICE_NULL_HANDLE)
  {
    IceLogError("Failed to create a material");
    return ICE_NULL_HANDLE;
  }

  return mat;
}

u32 Ice::zCreateLightingMaterial(std::vector<IceShader> _shaders)
{
  return renderer.CreateMaterial(_shaders, Ice_Mat_Deferred_Light, 1);
}

b8 Ice::zSetLightingMaterial(IceHandle _material)
{
  return renderer.SetLightingMaterial(_material);
}

void Ice::zAssignMaterialTextures(IceHandle _material,
                                              std::vector<IceTexture> _textures)
{
  renderer.AssignMaterialTextures(_material, _textures);
}

b8 Ice::zSetMaterialBufferData(IceHandle _material, void* _data)
{
  return renderer.SetMaterialBufferData(_material, _data);
}
