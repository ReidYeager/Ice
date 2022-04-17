
#ifndef ICE_CORE_SCENE_H_
#define ICE_CORE_SCENE_H_

#include "defines.h"

#include "core/ecs.h"
#include "math/vector.h"
#include "rendering/renderer_context.h"

namespace Ice {

  struct Transform
  {
    vec3 position = { 0.0f, 0.0f, 0.0f };
    // TODO : Convert Euler rotation to quaternion
    vec3 rotation = { 0.0f, 0.0f, 0.0f }; // Euler angles
    vec3 scale = { 1.0f, 1.0f, 1.0f };
  };

  struct TransformComponent
  {
    Ice::Transform transform;
    Ice::BufferSegment bufferSegment;
  };

  class Object
  {
  public:
    Ice::Transform* transform;

  protected:
    Ice::ECS::Entity id;

  public:
    Object(Ice::ECS::Entity _id) : id(_id) {}
    const Ice::ECS::Entity GetId() { return id; }
  };

  class Camera : public Object
  {
    
  };

}

#endif // !define ICE_CORE_SCENE_H_
