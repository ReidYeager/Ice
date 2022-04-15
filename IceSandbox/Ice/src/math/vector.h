
#ifndef ICE_MATH_VECTOR_H_
#define ICE_MATH_VECTOR_H_

#include "defines.h"

typedef struct vec2
{
  union { f32 x, r, width; };
  union { f32 y, g, height; };

  f32& operator[](int i)
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

  f32 const& operator[](int i) const
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

  bool operator==(vec2& other) const { return x == other.x && y == other.y; }
  bool operator==(const vec2& other) const { return x == other.x && y == other.y; }

} vec2;

typedef struct vec3
{
  union { f32 x, r; };
  union { f32 y, g; };
  union { f32 z, b; };

  f32& operator[](int i)
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

  f32 const& operator[](int i) const
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

  template<typename U>
  constexpr vec3& operator/=(U scalar)
  {
    x /= scalar;
    y /= scalar;
    z /= scalar;
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

  template<typename U>
  constexpr vec3& operator+=(vec3 other)
  {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
  }

  template<typename U>
  constexpr vec3& operator-=(vec3 other)
  {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
  }

  bool operator==(vec3& other) const { return x == other.x && y == other.y && z == other.z; }
  bool operator==(const vec3& other) const { return x == other.x && y == other.y && z == other.z; }

} vec3;

typedef struct vec4
{
  union { f32 x, r; };
  union { f32 y, g; };
  union { f32 z, b; };
  union { f32 w, a; };

  f32& operator[](int i)
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

  f32 const& operator[](int i) const
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

  template<typename U>
  constexpr vec4& operator+=(vec4 other)
  {
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;
    return *this;
  }

  template<typename U>
  constexpr vec4& operator-=(vec4 other)
  {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;
    return *this;
  }

  bool operator==(vec4& other) const
    { return x == other.x && y == other.y && z == other.z && w == other.w; }
  bool operator==(const vec4& other) const
    { return x == other.x && y == other.y && z == other.z && w == other.w; }

} vec4;

//=========================
// Int vectors
//=========================

typedef struct vec2I
{
  union { i32 x, r, width; };
  union { i32 y, g, height; };

  i32& operator[](int i)
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

  i32 const& operator[](int i) const
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

  i32& operator[](int i)
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

  i32 const& operator[](int i) const
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

  i32& operator[](int i)
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

  i32 const& operator[](int i) const
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

  u32& operator[](int i)
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

  u32 const& operator[](int i) const
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

  u32& operator[](int i)
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

  u32 const& operator[](int i) const
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

  u32& operator[](int i)
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

  u32 const& operator[](int i) const
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
