
#ifndef ICE_MATH_TRANSFORM_H_
#define ICE_MATH_TRANSFORM_H_

#include "defines.h"

#include "math/linear.h"

namespace Ice {

  inline quaternion EulerToQuaternion(vec3 _euler)
  {
    _euler *= 0.008726646f; // (Degrees to Radians)/2

    // Rotate round each local-space axis
    quaternion q1 = { 0, sin(_euler.y), 0, cos(_euler.y) }; // Yaw
    quaternion q2 = { sin(_euler.x), 0, 0, cos(_euler.x) }; // Pitch
    quaternion q3 = { 0, 0, sin(_euler.z), cos(_euler.z) }; // Roll

    // Combine into a quaternion rotated by q1, then q2, then q3
    return (q1 * q2 * q3).Normal();
  }

  inline quaternion EulerToQuaternion(f32 _x, f32 _y, f32 _z)
  {
    return EulerToQuaternion({_x, _y, _z});
  }

  class Transform
  {
  private:
    vec3 position;
    quaternion rotation;
    vec3 scale;

    mat4 matrix;
    Transform* parent;

    b8 dirty = false;

  public:

    Transform()
    {
      matrix = mat4Identity;
      scale = {1.0f, 1.0f, 1.0f};
      rotation = {0.0f, 0.0f, 0.0f, 1.0f};
    }

    // Position =====

    vec3 GetPosition()
    {
      return position;
    }

    // Directly sets the position
    vec3 SetPosition(vec3 _newPosition)
    {
      dirty = true;
      position = _newPosition;
      return position;
    }

    vec3 SetPosition(f32 _x, f32 _y, f32 _z)
    {
      return SetPosition({_x, _y, _z});
    }

    // Add this position onto the current position
    vec3 Translate(vec3 _translation)
    {
      dirty = true;
      position += _translation;
      return position;
    }

    vec3 Translate(f32 _x, f32 _y, f32 _z)
    {
      return Translate({_x, _y, _z});
    }

    // Rotation =====

    quaternion GetRotation()
    {
      return rotation;
    }

    // Set the current rotation to this Euler rotation
    quaternion SetRotation(vec3 _rotation)
    {
      dirty = true;
      rotation = Ice::EulerToQuaternion(_rotation);
      return rotation;
    }

    quaternion SetRotation(f32 _x, f32 _y, f32 _z)
    {
      return SetRotation({_x, _y, _z});
    }

    // Add this Euler rotation onto the current rotation
    quaternion Rotate(vec3 _rotation)
    {
      dirty = true;
      quaternion other = Ice::EulerToQuaternion(_rotation);

      // TODO : Figure out how to multiply quaternions to maintain the yaw-pitch-roll rotation order

      rotation *= other; // Rotates roll->pitch->yaw
      //rotation = other * rotation; // Rotates around world-space axes

      rotation.Normalize();
      return rotation;
    }

    quaternion Rotate(f32 _x, f32 _y, f32 _z)
    {
      return Rotate({_x, _y, _z});
    }

    // Scale =====

    vec3 GetScale()
    {
      return scale;
    }

    // Directly sets the scale
    vec3 SetScale(vec3 _newScale)
    {
      dirty = true;
      scale = _newScale;
      return scale;
    }

    vec3 SetScale(f32 _x, f32 _y, f32 _z)
    {
      return SetScale({_x, _y, _z});
    }

    // Add this scale onto the current scale
    vec3 Scale(vec3 _scale)
    {
      dirty = true;
      scale += _scale;
      return scale;
    }

    vec3 Scale(f32 _x, f32 _y, f32 _z)
    {
      return Scale({_x, _y, _z});
    }

    // Matrix =====

    mat4 GetMatrix(b8 _includeParents = true)
    {
      if (dirty)
      {
        dirty = false;

        vec3 p = position;
        quaternion q = rotation;
        vec3 s = scale;

        // Rotation only
        //matrix = {
        //  1.0f - 2.0f * (q.y * q.y + q.z * q.z), 2.0f * (q.x * q.y - q.w * q.z), 2.0f * (q.x * q.z + q.w * q.y), 0.0f,
        //  2.0f * (q.x * q.y + q.w * q.z), 1.0f - 2.0f * (q.x * q.x + q.z * q.z), 2.0f * (q.y * q.z - q.w * q.x), 0.0f,
        //  2.0f * (q.x * q.z - q.w * q.y), 2.0f * (q.y * q.z + q.w * q.x), 1.0f - 2.0f * (q.x * q.x + q.y * q.y), 0.0f,
        //  0.0f, 0.0f, 0.0f, 1.0f
        //};

        matrix = {
          s.x * (1 - 2 * (q.y * q.y + q.z * q.z)), s.y * (2 * (q.x * q.y - q.w * q.z))    , s.z * (2 * (q.x * q.z + q.w * q.y))    , p.x,
          s.x * (2 * (q.x * q.y + q.w * q.z))    , s.y * (1 - 2 * (q.x * q.x + q.z * q.z)), s.z * (2 * (q.y * q.z - q.w * q.x))    , p.y,
          s.x * (2 * (q.x * q.z - q.w * q.y))    , s.y * (2 * (q.y * q.z + q.w * q.x))    , s.z * (1 - 2 * (q.x * q.x + q.y * q.y)), p.z,
          0                                      , 0                                      , 0                                      , 1
        };
      }

      if (_includeParents && parent != nullptr)
      {
        return parent->GetMatrix() * matrix;
      }

      return matrix;
    }

    void SetParentAs(Transform* _newParent)
    {
      parent = _newParent;
    }

    Transform const* GetParent()
    {
      return parent;
    }

  };

}
#endif // !define ICE_MATH_TRANSFORM_H_
