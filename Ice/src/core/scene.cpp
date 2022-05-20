
#include "defines.h"

#include "core/scene.h"
#include "rendering/renderer.h"
#include "core/application.h" // createMesh

#include <iostream>

Ice::Scene::Scene(u32 _maxObjectCount /*= 100*/, u32 _maxComponentTypeCount /*= 10*/)
{
  maxObjectCount = _maxObjectCount;
  objects = (Ice::Object*)Ice::MemoryAllocZero(sizeof(Ice::Object) * maxObjectCount);
  componentsMap.reserve(_maxComponentTypeCount);

  // Initialize required component arrays
  componentsMap.insert(std::pair<size_t, void*>(typeid(Ice::TransformComponent).hash_code(),
                                                new Ice::ECS::ComponentManager<Ice::TransformComponent>));
  GetComponentManager<Ice::TransformComponent>()->Initialize(maxObjectCount);

  componentsMap.insert(std::pair<size_t, void*>(typeid(Ice::RenderComponent).hash_code(),
                                                new Ice::ECS::ComponentManager<Ice::RenderComponent>));
  GetComponentManager<Ice::RenderComponent>()->Initialize(maxObjectCount);

  componentsMap.insert(std::pair<size_t, void*>(typeid(Ice::CameraComponent).hash_code(),
                                                new Ice::ECS::ComponentManager<Ice::CameraComponent>));
  GetComponentManager<Ice::CameraComponent>()->Initialize(maxCameraCount);

  renderer->CreateBufferMemory(&transformsBuffer, 64, maxObjectCount, Ice::Buffer_Memory_Shader_Read);
}

Ice::Scene::~Scene()
{
  renderer->DestroyBufferMemory(&transformsBuffer);
  Ice::MemoryFree(objects);
  maxObjectCount = 0;

  for (std::pair<size_t, void*> p : componentsMap)
  {
    void* s = p.second;
    static_cast<Ice::ECS::ComponentManager<Ice::TransformComponent>*>(s)->Shutdown();
    delete(p.second);
  }
  componentsMap.clear();
}

void Ice::Scene::UpdateTransforms()
{
  Ice::ECS::ComponentManager<Ice::TransformComponent>& transformCm = *GetComponentManager<Ice::TransformComponent>();
  Ice::ECS::ComponentManager<Ice::CameraComponent>& camCm = *GetComponentManager<Ice::CameraComponent>();

  for (u32 i = 0; i < transformCm.GetCount(); i++)
  {
    Ice::CameraComponent* cam = camCm.GetComponent(transformCm.GetEntity(i));
    if (cam == nullptr)
    {
      //mat4 out = Ice::CalculateTransformMatrix(&tcs[i].transform).Transpose();
      mat4 out = transformCm[i].transform.GetMatrix(true).Transpose();
      renderer->PushDataToBuffer((void*)&out, transformCm[i].bufferSegment);
    }
    else
    {
      mat4 out;

      // Calculate camera transform =====
      {
        // Calculates a transform matrix that does the exact opposite of what the transform says
        vec3 s = transformCm[i].transform.GetScale();
        vec3 p = transformCm[i].transform.GetPosition();
        quaternion q = transformCm[i].transform.GetRotation();

        s = { 1.0f / s.x, 1.0f / s.y, 1.0f / s.z };
        p *= -1;
        q = { -q.x, -q.y, -q.z, q.w };
        q.Normalize();

        // Combine scale, rotation, position =====
        out = {
          2 * (q.w * q.w + q.x * q.x) - 1, 2 * (q.x * q.y - q.w * q.z)    , 2 * (q.w * q.y + q.x * q.z)    , 0,
          2 * (q.w * q.z + q.x * q.y)    , 2 * (q.w * q.w + q.y * q.y) - 1, 2 * (q.y * q.z - q.w * q.x)    , 0,
          2 * (q.x * q.z - q.w * q.y)    , 2 * (q.w * q.x + q.y * q.z)    , 2 * (q.w * q.w + q.z * q.z) - 1, 0,
          0                              , 0                              , 0                              , 0
        };

        out = {
          s.x * out.x.x, s.y * out.x.y, s.z * out.x.z, p.x * s.x * out.x.x + p.y * s.x * out.x.y + p.z * s.x * out.x.z,
          s.x * out.y.x, s.y * out.y.y, s.z * out.y.z, p.x * s.y * out.y.x + p.y * s.y * out.y.y + p.z * s.y * out.y.z,
          s.x * out.z.x, s.y * out.z.y, s.z * out.z.z, p.x * s.z * out.z.x + p.y * s.z * out.z.y + p.z * s.z * out.z.z,
          0            , 0            , 0            , 1
        };
      }

      out = cam->projectionMatrix * out; // Projection matrix * View matrix
      // TODO : Account for parent transform in camera matrix
      out = out.Transpose();

      renderer->PushDataToBuffer((void*)&out, transformCm[i].bufferSegment);
    }
  }
}

// TODO : Create a default mesh and default material for new objects to use
Ice::Object* Ice::Scene::AddObject(const char* _meshDir, Ice::Material* _material)
{
  if (objectCount >= maxObjectCount)
  {
    IceLogError("Scene already has max objects (%d)", maxObjectCount);
    return nullptr;
  }

  Ice::Object o(Ice::ECS::CreateEntity(), this);

  // Transform =====
  Ice::TransformComponent* tc = o.AddComponent<Ice::TransformComponent>();
  u32 index = GetComponentManager<Ice::TransformComponent>()->GetIndex(o.GetId());

  tc->bufferSegment.buffer = &transformsBuffer;
  tc->bufferSegment.elementSize = 64; // Should be defined somewhere else.
  tc->bufferSegment.count = 1;
  tc->bufferSegment.startIndex = index;
  renderer->PushDataToBuffer((void*)&mat4Identity, tc->bufferSegment);

  o.transform = &tc->transform;

  // Render component =====
  Ice::RenderComponent* rc = o.AddComponent<Ice::RenderComponent>();
  renderer->InitializeRenderComponent(rc, &tc->bufferSegment);
  Ice::CreateMesh(_meshDir, &rc->mesh);
  rc->material = _material;

  objects[objectCount] = o;
  objectCount++;

  return &objects[objectCount - 1];
}

Ice::Object* Ice::Scene::AddCamera(Ice::CameraSettings _settings /*= {}*/)
{
  if (GetComponentManager<Ice::CameraComponent>()->GetCount() >= maxCameraCount)
  {
    IceLogError("Scene already has max cameras (%d)", maxCameraCount);
    return nullptr;
  }

  if (objectCount >= maxObjectCount)
  {
    IceLogError("Scene already has max objects (%d)", maxObjectCount);
    return nullptr;
  }

  Ice::Object o(Ice::ECS::CreateEntity(), this);

  // Transform =====
  Ice::TransformComponent* tc = o.AddComponent<Ice::TransformComponent>();
  u32 index = GetComponentManager<Ice::TransformComponent>()->GetIndex(o.GetId());

  tc->bufferSegment.buffer = &transformsBuffer;
  tc->bufferSegment.elementSize = 64; // TODO : Define per-object buffer size somewhere else.
  tc->bufferSegment.count = 1;
  tc->bufferSegment.startIndex = index;
  renderer->PushDataToBuffer((void*)&mat4Identity, tc->bufferSegment);

  o.transform = &tc->transform;

  // Camera =====
  Ice::CameraComponent* cc = o.AddComponent<Ice::CameraComponent>();
  renderer->InitializeCamera(cc, tc->bufferSegment, _settings);

  objects[objectCount] = o;
  objectCount++;

  return &objects[objectCount - 1];
}
