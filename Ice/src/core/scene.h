
#ifndef ICE_CORE_SCENE_H_
#define ICE_CORE_SCENE_H_

#include "defines.h"

#include "core/ecs.h"
#include "math/vector.hpp"
#include "math/transform.h"
#include "rendering/renderer_defines.h"

#include <unordered_map>
#include <iostream>

namespace Ice {

  struct TransformComponent
  {
    Ice::Transform transform;
    Ice::BufferSegment bufferSegment;
  };

  class Object;

  class Scene
  {
    friend class Ice::Object;

  private:
    u8 maxCameraCount = 2; // TMP

    u32 maxObjectCount;
    u32 objectCount = 0;
    Ice::Object* objects;
    Ice::Buffer transformsBuffer; // Stores the transforms matrix for every object

    std::unordered_map<size_t, void*> componentsMap; // Stores pointers to ECS::ComponentManagers

  public:
    Scene(u32 _maxObjectCount, u32 _maxComponentTypeCount);
    ~Scene();

    void UpdateTransforms();
    Ice::Object* AddObject(const char* _meshDir, Ice::Material* _material);
    Ice::Object* AddCamera(Ice::CameraSettings _settings = {});

    template<typename T>
    constexpr Ice::ECS::ComponentManager<T>* GetComponentManager()
    {
      if (ContainsComponent<T>())
      {
        return static_cast<Ice::ECS::ComponentManager<T>*>(componentsMap[typeid(T).hash_code()]);
      }

      return nullptr;
    }

    template<typename T>
    constexpr b8 ContainsComponent()
    {
      return componentsMap.find(typeid(T).hash_code()) != componentsMap.end();
    }
  };

  class Object
  {
  public:
    Ice::Transform* transform;

  protected:
    Ice::ECS::Entity id;
    Ice::Scene* owningScene;

  public:
    Object(Ice::ECS::Entity _id, Ice::Scene* _scene) : id(_id), owningScene(_scene) {}
    const Ice::ECS::Entity GetId() { return id; }

    template<typename T>
    constexpr T* AddComponent()
    {
      if (!owningScene->ContainsComponent<T>())
      {
        owningScene->componentsMap.insert(std::pair<size_t, void*>(typeid(T).hash_code(),
                                                                   new Ice::ECS::ComponentManager<T>));

        owningScene->GetComponentManager<T>()->Initialize(owningScene->maxObjectCount);
      }

      T* component = owningScene->GetComponentManager<T>()->Create(id);
      return component;
    }

    template<typename T>
    constexpr T* GetComponent()
    {
      T* component = owningScene->GetComponentManager<T>()->GetComponent(id);
      return component;
    }

  };

}

#endif // !define ICE_CORE_SCENE_H_
