
#ifndef ICE_CORE_GAMEOBJECT_H_
#define ICE_CORE_GAMEOBJECT_H_

#include "defines.h"

#include "core/ecs_components.h"
#include "core/ecs_controller.h"

#include <cstdarg>

// Used to wrap ENTT calls for use in user applications
class ICE_API GameObject
{
private:
  entt::entity entity;
  IceEcsController* controller = nullptr;

public:
  GameObject() {}

  GameObject(IceEcsController* _controller)
  {
    controller = _controller;

    entity = controller->CreteEnity();
    controller->AddComponent<TransformComponent>(entity, 0.0f, 0.0f, 0.0f);
  }

  void Destroy()
  {
    controller->DestroyEntity(entity);
  }

  template <typename T, typename... Args>
  T& AddComponent(Args &&... args)
  {
    return controller->AddComponent<T>(entity, std::forward<Args>(args)...);
  }

  template <typename T>
  T& GetComponent()
  {
     return controller->GetComponent<T>(entity);
  }

  template <typename T>
  void RemoveComponent()
  {
    controller->RemoveComponent<T>(entity);
  }

};

#endif // !define ICE_CORE_GAMEOBJECT_H_
