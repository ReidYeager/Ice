
#ifndef ICE_CORE_SCENE_H_
#define ICE_CORE_SCENE_H_

#include "defines.h"

#include "core/ecs.h"
#include "math/vector.h"
#include "rendering/renderer_defines.h"

namespace Ice {

  struct Transform
  {
    vec3 position = { 0.0f, 0.0f, 0.0f };
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

  inline mat4 CalculateTransformMatrix(Ice::Transform* _transform)
  {
    vec3 s = _transform->scale;
    vec3 p = _transform->position;
    vec3 r = _transform->rotation * 0.008726646f; // (Degrees to Radians)/2

    // Calculate quaternion =====
    // Rotate round each local-space axis
    vec4 q1 = {0, sin(r.y), 0, cos(r.y)}; // Yaw
    vec4 q2 = {sin(r.x), 0, 0, cos(r.x)}; // Pitch
    vec4 q3 = {0, 0, sin(r.z), cos(r.z)}; // Roll

    // Combine into a quaternion rotated by q1, then q2, then q3
    vec4 q12 = {
      q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y, // X
      q1.w * q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z, // Y
      q1.w * q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x, // Z
      q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z  // W
    };
    vec4 q = {
      q12.w * q3.x + q12.x * q3.w + q12.y * q3.z - q12.z * q3.y, // X
      q12.w * q3.y + q12.y * q3.w + q12.z * q3.x - q12.x * q3.z, // Y
      q12.w * q3.z + q12.z * q3.w + q12.x * q3.y - q12.y * q3.x, // Z
      q12.w * q3.w - q12.x * q3.x - q12.y * q3.y - q12.z * q3.z  // W
    };

    // Combine scale, rotation, position =====
    return {
      s.x*(2*(q.w*q.w+q.x*q.x)-1), 2*s.y*(q.x*q.y-q.w*q.z)    , 2*s.z*(q.w*q.y+q.x*q.z)    , p.x,
      2*s.x*(q.w*q.z+q.x*q.y)    , s.y*(2*(q.w*q.w+q.y*q.y)-1), 2*s.z*(q.y*q.z-q.w*q.x)    , p.y,
      2*s.x*(q.x*q.z-q.w*q.y)    , 2*s.y*(q.w*q.x+q.y*q.z)    , s.z*(2*(q.w*q.w+q.z*q.z)-1), p.z,
      0                          , 0                          , 0                          , 1
    };
  }

  // Calculates a transform matrix that does the exact opposite of what the transform says
  inline mat4 CalculateCameraTransformMatrix(Ice::Transform* _transform)
  {
    vec3 s = _transform->scale;
    vec3 p = _transform->position;
    vec3 r = _transform->rotation * 0.008726646f; // (Degrees to Radians)/2

    s = { 1.0f / s.x, 1.0f / s.y, 1.0f / s.z };
    p *= -1;
    r *= -1;

    // Calculate quaternion =====
    // Rotate round each local-space axis
    vec4 q3 = {0, sin(r.y), 0, cos(r.y)}; // Yaw
    vec4 q2 = {sin(r.x), 0, 0, cos(r.x)}; // Pitch
    vec4 q1 = {0, 0, sin(r.z), cos(r.z)}; // Roll

    // Combine into a quaternion rotated by q1, then q2, then q3
    vec4 q12 = {
      q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y, // X
      q1.w * q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z, // Y
      q1.w * q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x, // Z
      q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z  // W
    };
    vec4 q = {
      q12.w * q3.x + q12.x * q3.w + q12.y * q3.z - q12.z * q3.y, // X
      q12.w * q3.y + q12.y * q3.w + q12.z * q3.x - q12.x * q3.z, // Y
      q12.w * q3.z + q12.z * q3.w + q12.x * q3.y - q12.y * q3.x, // Z
      q12.w * q3.w - q12.x * q3.x - q12.y * q3.y - q12.z * q3.z  // W
    };

    // Combine scale, rotation, position =====
    return {
      s.x*(2*(q.w*q.w+q.x*q.x)-1), 2*s.y*(q.x*q.y-q.w*q.z)    , 2*s.z*(q.w*q.y+q.x*q.z)    , s.x*p.x*(2*(q.w*q.w+q.x*q.x)-1)+2*s.x*p.y*(q.x*q.y-q.w*q.z)+2*s.x*p.z*(q.w*q.y+q.x*q.z),
      2*s.x*(q.w*q.z+q.x*q.y)    , s.y*(2*(q.w*q.w+q.y*q.y)-1), 2*s.z*(q.y*q.z-q.w*q.x)    , 2*s.y*p.x*(q.w*q.z+q.x*q.y)+s.y*p.y*(2*(q.w*q.w+q.y*q.y)-1)+2*s.y*p.z*(q.y*q.z-q.w*q.x),
      2*s.x*(q.x*q.z-q.w*q.y)    , 2*s.y*(q.w*q.x+q.y*q.z)    , s.z*(2*(q.w*q.w+q.z*q.z)-1), 2*s.z*p.x*(q.x*q.z-q.w*q.y)+2*s.z*p.y*(q.w*q.x+q.y*q.z)+s.z*p.z*(2*(q.w*q.w+q.z*q.z)-1),
      0                          , 0                          , 0                          , 1
    };
  }

  class Camera : public Object
  {
    
  };

}

#endif // !define ICE_CORE_SCENE_H_
