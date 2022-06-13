
#ifndef ICE_MATH_VECTOR_H_
#define ICE_MATH_VECTOR_H_

#include "defines.h"

#include "math.h"

typedef struct vec2
{
  union { f32 x, r, width; };
  union { f32 y, g, height; };

  constexpr f32& operator[](int i)
  {
    switch (i)
    {
    default:
    case 0:
      return x;
    case 1:
      return y;
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
    }
  }

  template<typename U>
  constexpr vec2& operator*=(U scalar)
  {
    x *= scalar;
    y *= scalar;
    return *this;
  }

  template<typename U>
  constexpr vec2& operator/=(U scalar)
  {
    x /= scalar;
    y /= scalar;
    return *this;
  }

  template<typename U>
  constexpr vec2& operator+=(U scalar)
  {
    x += scalar;
    y += scalar;
    return *this;
  }

  template<typename U>
  constexpr vec2& operator-=(U scalar)
  {
    x -= scalar;
    y -= scalar;
    return *this;
  }

  template<typename U>
  constexpr vec2& operator+=(vec2 other)
  {
    x += other.x;
    y += other.y;
    return *this;
  }

  template<typename U>
  constexpr vec2& operator-=(vec2 other)
  {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  template<typename U>
  constexpr vec2 operator*(U scalar)
  {
    return { x * scalar, y * scalar };
  }

  template<typename U>
  constexpr vec2 operator/(U scalar)
  {
    return { x / scalar, y / scalar };
  }

  template<typename U>
  constexpr vec2 operator+(U scalar)
  {
    return { x + scalar, y + scalar };
  }

  template<typename U>
  constexpr vec2 operator-(U scalar)
  {
    return { x - scalar, y - scalar };
  }

  template<typename U>
  constexpr vec2 operator+(vec2 other)
  {
    return { x + other.x, y + other.y };
  }

  template<typename U>
  constexpr vec2 operator-(vec2 other)
  {
    return { x - other.x, y - other.y };
  }

  bool operator==(vec2& other) const { return x == other.x && y == other.y; }
  bool operator==(const vec2& other) const { return x == other.x && y == other.y; }

  constexpr f32 Dot(vec2 other)
  {
    return x * other.x + y * other.y;
  }

  constexpr vec2 Normal()
  {
    return *this / (f32)sqrt(x * x + y * y);
  }

  constexpr vec2 Normalize()
  {
    *this /= (f32)sqrt(x * x + y * y);
    return *this;
  }

} vec2;

typedef struct vec3
{
  union { f32 x, r; };
  union { f32 y, g; };
  union { f32 z, b; };

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
    }
  }

  template<typename U>
  constexpr vec3& operator*=(U scalar)
  {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
  }

  constexpr vec3& operator*=(vec3 other)
  {
    x *= other.x;
    y *= other.y;
    z *= other.z;
    return *this;
  }

  template<typename U>
  constexpr vec3& operator/=(U scalar)
  {
    x /= scalar;
    y /= scalar;
    z /= scalar;
    return *this;
  }

  constexpr vec3& operator/=(vec3 other)
  {
    x /= other.x;
    y /= other.y;
    z /= other.z;
    return *this;
  }

  template<typename U>
  constexpr vec3& operator+=(U scalar)
  {
    x += scalar;
    y += scalar;
    z += scalar;
    return *this;
  }

  template<typename U>
  constexpr vec3& operator-=(U scalar)
  {
    x -= scalar;
    y -= scalar;
    z -= scalar;
    return *this;
  }

  constexpr vec3& operator+=(vec3 other)
  {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
  }

  constexpr vec3& operator-=(vec3 other)
  {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
  }

  template<typename U>
  constexpr vec3 operator*(U scalar)
  {
    return {x * scalar, y * scalar, z * scalar};
  }

  constexpr vec3 operator*(vec3 other)
  {
    return { x * other.x, y * other.y, z * other.z };
  }

  template<typename U>
  constexpr vec3 operator/(U scalar)
  {
    return {x / scalar, y / scalar, z / scalar};
  }

  constexpr vec3 operator/(vec3 other)
  {
    return { x / other.x, y / other.y, z / other.z };
  }

  template<typename U>
  constexpr vec3 operator+(U scalar)
  {
    return {x + scalar, y + scalar, z + scalar};
  }

  template<typename U>
  constexpr vec3 operator-(U scalar)
  {
    return {x - scalar, y - scalar, z - scalar};
  }

  constexpr vec3 operator+(vec3 other)
  {
    return {x + other.x, y + other.y, z + other.z};
  }

  constexpr vec3 operator-(vec3 other)
  {
    return {x - other.x, y - other.y, z - other.z};
  }

  bool operator==(vec3& other) const { return x == other.x && y == other.y && z == other.z; }
  bool operator==(const vec3& other) const { return x == other.x && y == other.y && z == other.z; }

  constexpr f32 Dot(vec3 other)
  {
    return x * other.x + y * other.y + z * other.z;
  }

  constexpr vec3 Cross(vec3 other)
  {
    return { (y * other.z) - (z * other.y),
             (z * other.x) - (x * other.z),
             (x * other.y) - (y * other.x) };
  }

  constexpr vec3 Normal()
  {
    return *this / (f32)sqrt(x * x + y * y + z * z);
  }

  constexpr vec3 Normalize()
  {
    *this /= (f32)sqrt(x * x + y * y + z * z);
    return *this;
  }

} vec3;

typedef struct vec4
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
  constexpr vec4& operator*=(U scalar)
  {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;
    return *this;
  }

  template<typename U>
  constexpr vec4& operator/=(U scalar)
  {
    x /= scalar;
    y /= scalar;
    z /= scalar;
    w /= scalar;
    return *this;
  }

  template<typename U>
  constexpr vec4& operator+=(U scalar)
  {
    x += scalar;
    y += scalar;
    z += scalar;
    w += scalar;
    return *this;
  }

  template<typename U>
  constexpr vec4& operator-=(U scalar)
  {
    x -= scalar;
    y -= scalar;
    z -= scalar;
    w -= scalar;
    return *this;
  }

  constexpr vec4& operator+=(vec4 other)
  {
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;
    return *this;
  }

  constexpr vec4& operator-=(vec4 other)
  {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;
    return *this;
  }

  template<typename U>
  constexpr vec4 operator*(U scalar)
  {
    return { x * scalar, y * scalar, z * scalar, w * scalar };
  }

  constexpr vec4 operator*(vec4 other)
  {
    return { x * other.x, y * other.y, z * other.z, w * other.w };
  }

  template<typename U>
  constexpr vec4 operator/(U scalar)
  {
    return { x / scalar, y / scalar, z / scalar, w / scalar };
  }

  template<typename U>
  constexpr vec4 operator+(U scalar)
  {
    return { x + scalar, y + scalar, z + scalar, w + scalar };
  }

  template<typename U>
  constexpr vec4 operator-(U scalar)
  {
    return { x - scalar, y - scalar, z - scalar, w - scalar };
  }

  template<typename U>
  constexpr vec4 operator+(vec4 other)
  {
    return { x + other.x, y + other.y, z + other.z, w + other.w };
  }

  template<typename U>
  constexpr vec4 operator-(vec4 other)
  {
    return { x - other.x, y - other.y, z - other.z, w - other.w };
  }

  constexpr bool operator==(vec4& other) const
    { return x == other.x && y == other.y && z == other.z && w == other.w; }
  constexpr bool operator==(const vec4& other) const
    { return x == other.x && y == other.y && z == other.z && w == other.w; }

  constexpr f32 Dot(vec4 other)
  {
    return x * other.x + y * other.y + z * other.z + w * other.w;
  }

  constexpr vec4 Normalized()
  {
    return *this / (f32)sqrt(x * x + y * y + z * z + w * w);
  }

  constexpr vec4 Normalize()
  {
    *this /= (f32)sqrt(x * x + y * y + z * z + w * w);
    return *this;
  }

  constexpr f32 Combined()
  {
    return x + y + z + w;
  }

} vec4;

//=========================
// Int vectors
//=========================

typedef struct vec2I
{
  union { i32 x, r, width; };
  union { i32 y, g, height; };

  constexpr i32& operator[](int i)
  {
    switch (i)
    {
    default:
    case 0:
      return x;
    case 1:
      return y;
    }
  }

  constexpr i32 const& operator[](int i) const
  {
    switch (i)
    {
    default:
    case 0:
      return x;
    case 1:
      return y;
    }
  }

} vec2I;

typedef struct vec3I
{
  union { i32 x, r; };
  union { i32 y, g; };
  union { i32 z, b; };

  constexpr i32& operator[](int i)
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
    }
  }

  constexpr i32 const& operator[](int i) const
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
    }
  }

} vec3I;

typedef struct vec4I
{
  union { i32 x, r; };
  union { i32 y, g; };
  union { i32 z, b; };
  union { i32 w, a; };

  constexpr i32& operator[](int i)
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

  constexpr i32 const& operator[](int i) const
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
} vec4I;

// =======================
// UInt vectors
// =======================

typedef struct vec2U
{
  union { u32 x, r, width; };
  union { u32 y, g, height; };

  constexpr u32& operator[](int i)
  {
    switch (i)
    {
    default:
    case 0:
      return x;
    case 1:
      return y;
    }
  }

  constexpr u32 const& operator[](int i) const
  {
    switch (i)
    {
    default:
    case 0:
      return x;
    case 1:
      return y;
    }
  }

} vec2U;

typedef struct vec3U
{
  union { u32 x, r; };
  union { u32 y, g; };
  union { u32 z, b; };

  constexpr u32& operator[](int i)
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
    }
  }

  constexpr u32 const& operator[](int i) const
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
    }
  }

} vec3U;

typedef struct vec4U
{
  union { u32 x, r; };
  union { u32 y, g; };
  union { u32 z, b; };
  union { u32 w, a; };

  constexpr u32& operator[](int i)
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

  constexpr u32 const& operator[](int i) const
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
} vec4U;

#endif // !define ICE_MATH_VECTOR_H_
