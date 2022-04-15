
#ifndef ICE_CORE_OBJECT_H_
#define ICE_CORE_OBJECT_H_

#include "defines.h"

#include "core/scene.h"
#include "core/ecs.h"

namespace Ice {

  class Object
  {
  public:
    Ice::Transform* transform;

  private:
    Ice::ECS::Entity id;

  public:
    Object(Ice::ECS::Entity _id) : id(_id) {}
    const Ice::ECS::Entity GetId() { return id; }

  };

}

#endif // !define ICE_CORE_OBJECT_H_
