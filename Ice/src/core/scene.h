
#ifndef ICE_CORE_SCENE_H_
#define ICE_CORE_SCENE_H_

#include "defines.h"

#include "core/ecs.h"
#include "math/vector.h"
#include "math/transform.h"
#include "rendering/renderer_defines.h"

#include "math.h"

namespace Ice {

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

  // Calculates a transform matrix that does the exact opposite of what the transform says
  inline mat4 CalculateCameraTransformMatrix(Ice::Transform* _transform)
  {
    vec3 s = _transform->GetScale();
    vec3 p = _transform->GetPosition();
    quaternion q = _transform->GetRotation();

    s = { 1.0f / s.x, 1.0f / s.y, 1.0f / s.z };
    p *= -1;
    q = {-q.x, -q.y, -q.z, q.w};
    q.Normalize();

    // Combine scale, rotation, position =====
    mat4 out = {
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
    return out;
  }

}

#endif // !define ICE_CORE_SCENE_H_
