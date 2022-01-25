
#include "defines.h"
#include "logger.h"

#include "core/application.h"
#include "core/input.h"
#include "core/object.h"

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <chrono>

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
  if (!reRenderer.Initialize(_settings->rendererSettings))
  {
    IceLogFatal("Ice Renderer failed to initialize");
    return false;
  }

  // NOTE : Can not init a lighting material with CreateMaterial because it takes subpassInputs
  // Need to include subpassInputs in shader descriptor parsing
  //reRenderer.CreateMaterial({ { "_light_blank", Ice_Shader_Vertex },
  //                            { _settings->rendererSettings.lightingShader, Ice_Shader_Fragment }});

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
    state.ClientUpdate(deltaTime);

    ICE_ATTEMPT(reRenderer.Render(&cam));

    // Update timing =====
    {
      end = std::chrono::steady_clock::now();
      microsecDelta = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
      deltaTime = microsecDelta.count() * microToSecond;
      start = end;

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

  reRenderer.Shutdown();
  rePlatform.Shutdown();
  return true;
}

IceObject* reIceApplication::AddObject(const char* _meshDir, u32 _material, IceObject* _parent)
{
  IceObject* obj = new IceObject();
  obj->materialHandle = _material;
  obj->meshHandle = reRenderer.CreateMesh(_meshDir);

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

  reRenderer.AddObjectToScene(obj);
  obj->transform.UpdateMatrix();

  return obj;
}

u32 reIceApplication::CreateMaterial(std::vector<IceShader> _shaders)
{
  return reRenderer.CreateMaterial(_shaders);
}

void reIceApplication::AssignMaterialTextures(IceHandle _material, std::vector<std::string> _images)
{
  reRenderer.AssignMaterialTextures(_material, _images);
}

glm::mat4 IceTransform::UpdateMatrix()
{
  matrix = parent->matrix;

  matrix = glm::translate(matrix, position);
  matrix = glm::rotate(matrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
  matrix = glm::rotate(matrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
  matrix = glm::rotate(matrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
  matrix = glm::scale(matrix, scale);

  reRenderer.FillBuffer(&buffer, &matrix, 64);

  // TODO : Update children's matrices

  return matrix;
}
