
#ifndef ICE_CORE_SCENE_H_
#define ICE_CORE_SCENE_H_

#include "defines.h"

#include "core/ecs.h"
#include "math/vector.h"

namespace Ice {

  struct Transform
  {
    vec3 position;
    vec3 rotation;
    vec3 scale;
  };

  struct Entity
  {
    const Ice::ECS::Entity id;
    Ice::Transform transform;
  };

}


#endif // !define ICE_CORE_SCENE_H_
