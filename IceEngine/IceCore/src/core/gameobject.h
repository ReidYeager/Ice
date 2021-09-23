
#ifndef ICE_CORE_GAMEOBJECT_H_
#define ICE_CORE_GAMEOBJECT_H_

#include "defines.h"

#include "core/ecs_controller.h"

struct dummyComponent
{
  int x;
  float y;
  char z;
};

class ICE_API GameObject
{
private:
  entt::entity entity;
  IceEcsController* controller;

public:
  GameObject(IceEcsController* _controller)
  {
    controller = _controller;

    entity = controller->CreteEnity();
  }

  template <typename T>
  T& AddComponent()
  {
    return controller->AddComponent<T>(entity);
  }

  template <typename T>
  T& GetComponent()
  {
     return controller->GetComponent<dummyComponent>(entity);
  }

  template <typename T>
  void RemoveComponent()
  {
    controller->RemoveComponent<dummyComponent>(entity);
  }

};

#endif // !define ICE_CORE_GAMEOBJECT_H_
