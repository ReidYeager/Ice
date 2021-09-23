
#ifndef ICE_CORE_ECS_CONTROLLER_H_
#define ICE_CORE_ECS_CONTROLLER_H_

#include "defines.h"

#include <entt/entt.hpp>

class ICE_API IceEcsController
{
private:
  entt::registry registry;

public:
  //IceEcsController() = delete;
  //IceEcsController(IceEcsController&&) = delete;
  //IceEcsController(const IceEcsController&) = delete;
  //void* operator new() = delete;

  entt::entity CreteEnity()
  {
    return registry.create();
  }

  template <typename T>
  T& AddComponent(const entt::entity& _entity)
  {
    return registry.get_or_emplace<T>(_entity);
  }

  template<typename T>
  T& GetComponent(const entt::entity& _entity)
  {
    return registry.get_or_emplace<T>(_entity);
  }

  template <typename T>
  void RemoveComponent(const entt::entity& _entity)
  {
    registry.remove<T>(_entity);
  }

};

#endif // !define ICE_CORE_ECS_CONTROLLER_H_
