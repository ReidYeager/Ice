
#ifndef ICE_MATH_QUATERNION_H_
#define ICE_MATH_QUATERNION_H_

#include "defines.h"
#include "math.h"

typedef struct quaternion
{
  union { f32 x, r; };
  union { f32 y, g; };
  union { f32 z, b; };
  union { f32 w, a; };

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

  template<typename U>
  constexpr quaternion& operator*=(U scalar)
  {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;
    return *this;
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

  template<typename U>
  constexpr quaternion& operator+=(U scalar)
  {
    x += scalar;
    y += scalar;
    z += scalar;
    w += scalar;
    return *this;
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

  constexpr quaternion& operator+=(vec4 other)
  {
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;
    return *this;
  }

  constexpr quaternion& operator-=(vec4 other)
  {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;
    return *this;
  }

  constexpr quaternion& operator*=(quaternion other)
  {
    x = w * other.x + x * other.w + y * other.z - z * other.y;
    y = w * other.y + y * other.w + z * other.x - x * other.z;
    z = w * other.z + z * other.w + x * other.y - y * other.x;
    w = w * other.w - x * other.x - y * other.y - z * other.z;

    return *this;
  };


  template<typename U>
  constexpr quaternion operator*(U scalar)
  {
    return { x * scalar, y * scalar, z * scalar, w * scalar };
  }

  constexpr quaternion operator*(quaternion other)
  {
    return { w * other.x + x * other.w + y * other.z - z * other.y, // X
             w * other.y + y * other.w + z * other.x - x * other.z, // Y
             w * other.z + z * other.w + x * other.y - y * other.x, // Z
             w * other.w - x * other.x - y * other.y - z * other.z  // W
    };
  }

  template<typename U>
  constexpr quaternion operator/(U scalar)
  {
    return { x / scalar, y / scalar, z / scalar, w / scalar };
  }

  template<typename U>
  constexpr quaternion operator+(U scalar)
  {
    return { x + scalar, y + scalar, z + scalar, w + scalar };
  }

  template<typename U>
  constexpr quaternion operator-(U scalar)
  {
    return { x - scalar, y - scalar, z - scalar, w - scalar };
  }

  template<typename U>
  constexpr quaternion operator+(vec4 other)
  {
    return { x + other.x, y + other.y, z + other.z, w + other.w };
  }

  template<typename U>
  constexpr quaternion operator-(vec4 other)
  {
    return { x - other.x, y - other.y, z - other.z, w - other.w };
  }

  constexpr bool operator==(quaternion& other) const
    { return x == other.x && y == other.y && z == other.z && w == other.w; }
  constexpr bool operator==(const quaternion& other) const
    { return x == other.x && y == other.y && z == other.z && w == other.w; }

  constexpr f32 Dot(quaternion other)
  {
    return x * other.x + y * other.y + z * other.z + w * other.w;
  }

  constexpr quaternion Normal()
  {
    return *this / (f32)sqrt(x * x + y * y + z * z + w * w);
  }

  constexpr quaternion Normalize()
  {
    *this /= (f32)sqrt(x * x + y * y + z * z + w * w);
    return *this;
  }

} quaternion;

#endif // !define ICE_MATH_QUATERNION_H_
