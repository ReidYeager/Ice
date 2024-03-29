
#ifndef ICE_MATH_QUATERNION_H_
#define ICE_MATH_QUATERNION_H_

#include "defines.h"
#include "math/vector.h"
#include "math/matrix.hpp"

#include "math.h"

namespace Ice {

typedef struct quaternion
{
  f32 x = 0;
  f32 y = 0;
  f32 z = 0;
  f32 w = 1;

  constexpr quaternion(f32 _x, f32 _y, f32 _z, f32 _w) : x(_x), y(_y), z(_z), w(_w) {}
  constexpr quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}

  constexpr f32& operator[](int i)
  {
    switch (i)
    {
    default:
    case 0:
      return x;
    case 1:
      return y;
    case 2:
      return z;
    case 3:
      return w;
    }
  }

  constexpr f32 const& operator[](int i) const
  {
    switch (i)
    {
    default:
    case 0:
      return x;
    case 1:
      return y;
    case 2:
      return z;
    case 3:
      return w;
    }
  }

  //=========================
  // Operators
  //=========================

  constexpr bool operator==(quaternion& other) const
  {
    return x == other.x && y == other.y && z == other.z && w == other.w;
  }
  constexpr bool operator==(const quaternion& other) const
  {
    return x == other.x && y == other.y && z == other.z && w == other.w;
  }

  //=========================
  // Mathematic Operators
  //=========================

  // Multiplication =====

  template<typename U>
  constexpr quaternion operator*(U scalar)
  {
    return { x * scalar, y * scalar, z * scalar, w * scalar };
  }

  template<typename U>
  constexpr quaternion& operator*=(U scalar)
  {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;
    return *this;
  }

  constexpr vec3 operator*(vec3 vec)
  {
    return Matrix() * vec;
  }

  constexpr quaternion operator*(quaternion other)
  {
    return
    {
      w * other.x + x * other.w + y * other.z - z * other.y, // X
      w * other.y + y * other.w + z * other.x - x * other.z, // Y
      w * other.z + z * other.w + x * other.y - y * other.x, // Z
      w * other.w - x * other.x - y * other.y - z * other.z  // W
    };
  }

  constexpr quaternion& operator*=(quaternion other)
  {
    x = w * other.x + x * other.w + y * other.z - z * other.y;
    y = w * other.y + y * other.w + z * other.x - x * other.z;
    z = w * other.z + z * other.w + x * other.y - y * other.x;
    w = w * other.w - x * other.x - y * other.y - z * other.z;
    return *this;
  };

  // Division =====

  template<typename U>
  constexpr quaternion operator/(U scalar)
  {
    return { x / scalar, y / scalar, z / scalar, w / scalar };
  }

  template<typename U>
  constexpr quaternion& operator/=(U scalar)
  {
    x /= scalar;
    y /= scalar;
    z /= scalar;
    w /= scalar;
    return *this;
  }

  // Addition =====

  template<typename U>
  constexpr quaternion operator+(U scalar)
  {
    return { x + scalar, y + scalar, z + scalar, w + scalar };
  }

  template<typename U>
  constexpr quaternion& operator+=(U scalar)
  {
    x += scalar;
    y += scalar;
    z += scalar;
    w += scalar;
    return *this;
  }

  constexpr quaternion operator+(vec4 other)
  {
    return { x + other.x, y + other.y, z + other.z, w + other.w };
  }

  constexpr quaternion& operator+=(vec4 other)
  {
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;
    return *this;
  }

  // Subtraction =====

  template<typename U>
  constexpr quaternion operator-(U scalar)
  {
    return { x - scalar, y - scalar, z - scalar, w - scalar };
  }

  template<typename U>
  constexpr quaternion& operator-=(U scalar)
  {
    x -= scalar;
    y -= scalar;
    z -= scalar;
    w -= scalar;
    return *this;
  }

  constexpr quaternion operator-(vec4 other)
  {
    return { x - other.x, y - other.y, z - other.z, w - other.w };
  }

  constexpr quaternion& operator-=(vec4 other)
  {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;
    return *this;
  }

  //=========================
  // Operations
  //=========================

  //constexpr f32 Combined()
  //{
  //  return x + y + z + w;
  //}

  constexpr f32 Dot(quaternion other)
  {
    return x * other.x + y * other.y + z * other.z + w * other.w;
  }

  constexpr quaternion Inverse()
  {
    return { -x, -y, -z, w };
  }

  constexpr quaternion& Invert()
  {
    x *= -1;
    y *= -1;
    z *= -1;
    return *this;
  }

  mat4 Matrix()
  {
    return Ice::mat4(2.0f*(w*w + x*x) - 1.0f, 2.0f*(x*y - w*z)       , 2.0f*(x*z + w*y)       , 0.0f,
                     2.0f*(x*y + w*z)       , 2.0f*(w*w + y*y) - 1.0f, 2.0f*(y*z - w*x)       , 0.0f,
                     2.0f*(x*z - w*y)       , 2.0f*(y*z + w*x)       , 2.0f*(w*w + z*z) - 1.0f, 0.0f,
                     0.0f                   , 0.0f                   , 0.0f                   , 1.0f,
                     true);
  }

  constexpr quaternion Normal()
  {
    return *this / (f32)sqrt(x * x + y * y + z * z + w * w);
  }

  constexpr quaternion& Normalize()
  {
    *this /= (f32)sqrt(x * x + y * y + z * z + w * w);
    return *this;
  }

} quaternion;

} // namespace Ice

#endif // !define ICE_MATH_QUATERNION_H_
