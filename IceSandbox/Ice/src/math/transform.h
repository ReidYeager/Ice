
#ifndef ICE_MATH_TRANSFORM_H_
#define ICE_MATH_TRANSFORM_H_

#include "defines.h"

#include "math/vector.h"
#include "math/matrix.h"

namespace Ice {

  class Transform
  {
  private:
    vec3 position;
    quaternion rotation;
    vec3 scale;

    mat4 matrix;
    Transform* parent;

    b8 dirty = false;

    quaternion EulerToQuaternion(vec3 _euler)
    {
      _euler *= 0.008726646f; // (Degrees to Radians)/2

      // Rotate round each local-space axis
      vec4 q1 = { 0, sin(_euler.y), 0, cos(_euler.y) }; // Yaw
      vec4 q2 = { sin(_euler.x), 0, 0, cos(_euler.x) }; // Pitch
      vec4 q3 = { 0, 0, sin(_euler.z), cos(_euler.z) }; // Roll

      // Combine into a quaternion rotated by q1, then q2, then q3
      vec4 q12 = {
        q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y, // X
        q1.w * q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z, // Y
        q1.w * q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x, // Z
        q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z  // W
      };
      return {
        q12.w * q3.x + q12.x * q3.w + q12.y * q3.z - q12.z * q3.y, // X
        q12.w * q3.y + q12.y * q3.w + q12.z * q3.x - q12.x * q3.z, // Y
        q12.w * q3.z + q12.z * q3.w + q12.x * q3.y - q12.y * q3.x, // Z
        q12.w * q3.w - q12.x * q3.x - q12.y * q3.y - q12.z * q3.z  // W
      };
    }

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

    // Add this position onto the current position
    vec3 Translate(vec3 _translation)
    {
      dirty = true;
      position += _translation;
      return position;
    }

    // Rotation =====

    quaternion GetRotation()
    {
      return rotation;
    }

    // vec3 GetRotationEuler()

    // Set the current rotation to this Euler rotation
    quaternion SetRotation(vec3 _rotation)
    {
      dirty = true;
      rotation = EulerToQuaternion(_rotation);
      return rotation;
    }

    // Add this Euler rotation onto the current rotation
    quaternion Rotate(vec3 _rotation)
    {
      dirty = true;
      quaternion other = EulerToQuaternion(_rotation);

      //other = { -other.x, -other.y, -other.z, other.w } / sqrt(other.x * other.x + other.y * other.y + other.z * other.z + other.w * other.w);
      //other = { -other.x, -other.y, -other.z, other.w };

      // Rotates only along world-space axis
      //rotation = {
      //   other.x * rotation.w + other.y * rotation.z - other.z * rotation.y + other.w * rotation.x,
      //  -other.x * rotation.z + other.y * rotation.w + other.z * rotation.x + other.w * rotation.y,
      //   other.x * rotation.y - other.y * rotation.x + other.z * rotation.w + other.w * rotation.z,
      //  -other.x * rotation.x - other.y * rotation.y - other.z * rotation.z + other.w * rotation.w
      //};

      // NOTE : Rotates in the wrong order : roll -> pitch -> yaw (Should be yaw -> pitch -> roll)
      rotation = {
        rotation.x * other.w + rotation.y * other.z - rotation.z * other.y + rotation.w * other.x,
       -rotation.x * other.z + rotation.y * other.w + rotation.z * other.x + rotation.w * other.y,
        rotation.x * other.y - rotation.y * other.x + rotation.z * other.w + rotation.w * other.z,
       -rotation.x * other.x - rotation.y * other.y - rotation.z * other.z + rotation.w * other.w
      };

      // The above two quaternions are each-other's inverse

      // Normalize
      //rotation /= sqrt(rotation.x * rotation.x + rotation.y * rotation.y + rotation.z * rotation.z + rotation.w * rotation.w);

      return rotation;
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

    // Add this scale onto the current scale
    vec3 Scale(vec3 _scale)
    {
      dirty = true;
      scale += _scale;
      return scale;
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

        // Inverts the quaternion : (world-axis rotations -> roll/pitch/yaw rotations) or vice-versa
        //q = { -q.x, -q.y, -q.z, q.w } / sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);

        matrix = {
          s.x*(2*(q.w*q.w+q.x*q.x)-1), s.y*(2*(q.x*q.y-q.w*q.z))  , s.z*(2*(q.w*q.y+q.x*q.z))  , p.x,
          s.x*(2*(q.w*q.z+q.x*q.y))  , s.y*(2*(q.w*q.w+q.y*q.y)-1), s.z*(2*(q.y*q.z-q.w*q.x))  , p.y,
          s.x*(2*(q.x*q.z-q.w*q.y))  , s.y*(2*(q.w*q.x+q.y*q.z))  , s.z*(2*(q.w*q.w+q.z*q.z)-1), p.z,
          0                          , 0                          , 0                          , 1
        };

        //matrix = {
        //  s.x*(q.w*q.w+q.x*q.x-q.y*q.y-q.z*q.z), 2*s.y*(q.x*q.y-q.w*q.z)    , 2*s.z*(q.w*q.y+q.x*q.z)    , p.x,
        //  2*s.x*(q.w*q.z+q.x*q.y)    , s.y*(q.w*q.w-q.x*q.x+q.y*q.y-q.z*q.z), 2*s.z*(q.y*q.z-q.w*q.x)    , p.y,
        //  2*s.x*(q.x*q.z-q.w*q.y)    , 2*s.y*(q.w*q.x+q.y*q.z)    , s.z*(q.w*q.w-q.x*q.x-q.y*q.y+q.z*q.z), p.z,
        //  0                          , 0                          , 0                          , 1
        //};
      }

      if (_includeParents && parent != nullptr)
      {
        return parent->GetMatrix() * matrix;
      }

      return matrix;
    }

  };

}
#endif // !define ICE_MATH_TRANSFORM_H_
