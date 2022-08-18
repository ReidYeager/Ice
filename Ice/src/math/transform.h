
#ifndef ICE_MATH_TRANSFORM_H_
#define ICE_MATH_TRANSFORM_H_

#include "defines.h"

#include "math/linear.h"

namespace Ice {

inline quaternion EulerToQuaternion(Ice::vec3 _euler)
{
  _euler *= 0.008726646f; // (Degrees to Radians)/2

  // Rotate round each local-space axis
  Ice::quaternion q1 = { 0.0f         , (f32)sin(_euler.y), 0.0f         , (f32)cos(_euler.y) }; // Yaw
  Ice::quaternion q2 = { (f32)sin(_euler.x), 0.0f         , 0.0f         , (f32)cos(_euler.x) }; // Pitch
  Ice::quaternion q3 = { 0.0f         , 0.0f         , (f32)sin(_euler.z), (f32)cos(_euler.z) }; // Roll

  // Combine into a quaternion rotated by q1, then q2, then q3
  return (q1 * q2 * q3).Normal();
}

inline quaternion EulerToQuaternion(f32 _x, f32 _y, f32 _z)
{
  return EulerToQuaternion({ _x, _y, _z });
}

class Transform
{
private:
  Ice::vec3 position;
  Ice::quaternion rotation;
  Ice::vec3 scale;

  mat4 matrix;
  Transform* parent = nullptr;

  b8 dirty = false;

public:

  Ice::BufferSegment bufferSegment;

  Transform()
  {
    matrix = mat4Identity;
    position = Ice::vec3(0.0f, 0.0f, 0.0f);
    scale = Ice::vec3(1.0f, 1.0f, 1.0f);
    rotation = Ice::quaternion(0.0f, 0.0f, 0.0f, 1.0f);
  }

  // Position =====

  constexpr Ice::vec3 GetPosition()
  {
    if (parent != nullptr)
    {
      vec4 pos = (parent->GetMatrix() * vec4({ position.x, position.y, position.z, 1.0f }));
      return Ice::vec3({ pos.x, pos.y, pos.z });
    }
    return position;
  }

  // Directly sets the position
  Ice::vec3 SetPosition(Ice::vec3 _newPosition)
  {
    dirty = true;
    position = _newPosition;
    return position;
  }

  Ice::vec3 SetPosition(f32 _x, f32 _y, f32 _z)
  {
    return SetPosition({ _x, _y, _z });
  }

  // Add this position onto the current position
  Ice::vec3 Translate(Ice::vec3 _translation)
  {
    dirty = true;
    position += _translation;
    return position;
  }

  Ice::vec3 Translate(f32 _x, f32 _y, f32 _z)
  {
    return Translate({ _x, _y, _z });
  }

  // Rotation =====

  constexpr quaternion GetRotation()
  {
    if (parent != nullptr)
    {
      return parent->GetRotation() * rotation;
    }
    return rotation;
  }

  // Set the current rotation to this Euler rotation
  quaternion SetRotation(quaternion _q)
  {
    dirty = true;
    rotation = _q;
    return rotation;
  }

  quaternion SetEulerRotation(Ice::vec3 _rotation)
  {
    return SetRotation(Ice::EulerToQuaternion(_rotation));
  }

  quaternion SetEulerRotation(f32 _x, f32 _y, f32 _z)
  {
    return SetRotation(Ice::EulerToQuaternion(_x, _y, _z));
  }

  // Add this Euler rotation onto the current rotation
  quaternion Rotate(quaternion _rotation)
  {
    dirty = true;

    rotation *= _rotation; // Rotates roll->pitch->yaw
    //rotation = _rotation * rotation; // Rotates around world-space axes

    rotation.Normalize();
    return rotation;
  }

  quaternion RotateEuler(Ice::vec3 _rotation)
  {
    return Rotate(Ice::EulerToQuaternion(_rotation.x, _rotation.y, _rotation.z));
  }

  quaternion RotateEuler(f32 _x, f32 _y, f32 _z)
  {
    return Rotate(Ice::EulerToQuaternion(_x, _y, _z));
  }

  // Scale =====

  constexpr Ice::vec3 GetScale()
  {
    if (parent != nullptr)
    {
      return parent->GetScale() * scale;
    }
    return scale;
  }

  // Directly sets the scale
  Ice::vec3 SetScale(Ice::vec3 _newScale)
  {
    dirty = true;
    scale = _newScale;
    return scale;
  }

  Ice::vec3 SetScale(f32 _x, f32 _y, f32 _z)
  {
    return SetScale({ _x, _y, _z });
  }

  // Add this scale onto the current scale
  Ice::vec3 Scale(Ice::vec3 _scale)
  {
    dirty = true;
    scale += _scale;
    return scale;
  }

  Ice::vec3 Scale(f32 _x, f32 _y, f32 _z)
  {
    return Scale({ _x, _y, _z });
  }

  // Matrix =====

  mat4 GetMatrix(b8 _includeParents = true)
  {
    if (dirty)
    {
      dirty = false;

      Ice::vec3 p = position;
      quaternion q = rotation;
      Ice::vec3 s = scale;

      matrix = Ice::mat4(
        s.x * (1 - 2 * (q.y * q.y + q.z * q.z)), s.x * (2 * (q.x * q.y + q.w * q.z))    , s.x * (2 * (q.x * q.z - q.w * q.y))    , 0,
        s.y * (2 * (q.x * q.y - q.w * q.z))    , s.y * (1 - 2 * (q.x * q.x + q.z * q.z)), s.y * (2 * (q.y * q.z + q.w * q.x))    , 0,
        s.z * (2 * (q.x * q.z + q.w * q.y))    , s.z * (2 * (q.y * q.z - q.w * q.x))    , s.z * (1 - 2 * (q.x * q.x + q.y * q.y)), 0,
        p.x                                    , p.y                                    , p.z                                    , 1
      );
    }

    if (_includeParents && parent != nullptr)
    {
      return parent->GetMatrix() * matrix;
    }

    return matrix;
  }

  constexpr void SetParentAs(Transform* _newParent)
  {
    parent = _newParent;
  }

  constexpr b8 HasParent()
  {
    return parent != nullptr;
  }

  constexpr Transform const* GetParent()
  {
    return parent;
  }

  // Vectors =====

  constexpr Ice::vec3 ForwardVector(b8 _includeParents = true)
  {
    return (rotation.Matrix() * Ice::vec3(0.0f, 0.0f, -1.0f)).Normal();
  }

  constexpr Ice::vec3 RightVector(b8 _includeParents = true)
  {
    return (rotation.Matrix() * Ice::vec3(1.0f, 0.0f, 0.0f)).Normal();
  }

  constexpr Ice::vec3 UpVector(b8 _includeParents = true)
  {
    return (rotation.Matrix() * Ice::vec3(0.0f, 1.0f, 0.0f)).Normal();
  }

};

} // namespace Ice

#endif // !define ICE_MATH_TRANSFORM_H_
