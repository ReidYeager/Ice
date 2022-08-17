
#ifndef ICE_MATH_VECTOR_H_
#define ICE_MATH_VECTOR_H_

#include "defines.h"

#include <math.h>
#include <assert.h>

namespace Ice {

//=========================
//=========================
// F32
//=========================
//=========================

typedef struct vec2
{
  union { f32 x, r, width, pitch; };
  union { f32 y, g, height, yaw; };

  constexpr vec2(f32 _x, f32 _y) : x(_x), y(_y) {}
  constexpr vec2(f32 _val) : x(_val), y(_val) {}
  constexpr vec2() : x(0.0f), y(0.0f) {}

  //=========================
  // Operators
  //=========================

  constexpr f32& operator[](int i)
  {
    assert(i < 2);

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
    assert(i < 2);

    switch (i)
    {
    default:
    case 0:
      return x;
    case 1:
      return y;
    }
  }

  constexpr bool operator==(vec2& other) const
  {
    return x == other.x && y == other.y;
  }

  constexpr bool operator==(const vec2& other) const
  {
    return x == other.x && y == other.y;
  }

  //=========================
  // Mathematic Operators
  //=========================

  // Multiplication =====

  template<typename U>
  constexpr vec2 operator*(U scalar)
  {
    return { x * scalar, y * scalar };
  }

  template<typename U>
  constexpr vec2& operator*=(U scalar)
  {
    x *= scalar;
    y *= scalar;
    return *this;
  }

  constexpr vec2 operator*(vec2 other)
  {
    return { x * other.x, y * other.y };
  }

  // Division =====

  template<typename U>
  constexpr vec2 operator/(U scalar)
  {
    return { x / scalar, y / scalar };
  }

  template<typename U>
  constexpr vec2& operator/=(U scalar)
  {
    x /= scalar;
    y /= scalar;
    return *this;
  }

  // Addition =====

  template<typename U>
  constexpr vec2 operator+(U scalar)
  {
    return { x + scalar, y + scalar };
  }

  template<typename U>
  constexpr vec2& operator+=(U scalar)
  {
    x += scalar;
    y += scalar;
    return *this;
  }

  constexpr vec2 operator+(vec2 other)
  {
    return { x + other.x, y + other.y };
  }

  constexpr vec2& operator+=(vec2 other)
  {
    x += other.x;
    y += other.y;
    return *this;
  }

  // Subtraction =====

  template<typename U>
  constexpr vec2 operator-(U scalar)
  {
    return { x - scalar, y - scalar };
  }

  template<typename U>
  constexpr vec2& operator-=(U scalar)
  {
    x -= scalar;
    y -= scalar;
    return *this;
  }

  constexpr vec2 operator-(vec2 other)
  {
    return { x - other.x, y - other.y };
  }

  constexpr vec2& operator-=(vec2 other)
  {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  //=========================
  // Operations
  //=========================

  //constexpr f32 Combined()
  //{
  //  return x + y + z + w;
  //}

  constexpr f32 Dot(vec2 other)
  {
    return x * other.x + y * other.y;
  }

  constexpr vec2 Normal()
  {
    if (x == 0.0f && y == 0.0f)
      return *this;

    return *this / (f32)sqrt(x * x + y * y);
  }

  constexpr vec2& Normalize()
  {
    if (x == 0.0f && y == 0.0f)
      return *this;

    *this /= (f32)sqrt(x * x + y * y);
    return *this;
  }

} vec2;

typedef struct vec3
{
  union { f32 x, r; };
  union { f32 y, g; };
  union { f32 z, b; };

  constexpr vec3(f32 _x, f32 _y, f32 _z) : x(_x), y(_y), z(_z) {}
  constexpr vec3(f32 _val) : x(_val), y(_val), z(_val) {}
  constexpr vec3() : x(0.0f), y(0.0f), z(0.0f) {}

  //=========================
  // Operators
  //=========================

  constexpr f32& operator[](int i)
  {
    assert(i < 3);

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
    assert(i < 3);

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

  constexpr bool operator==(vec3& other) const
  {
    return x == other.x && y == other.y && z == other.z;
  }

  constexpr bool operator==(const vec3& other) const
  {
    return x == other.x && y == other.y && z == other.z;
  }

  //=========================
  // Mathematic Operators
  //=========================

  // Multiplication =====

  template<typename U>
  constexpr vec3 operator*(U scalar)
  {
    return { x * scalar, y * scalar, z * scalar };
  }

  template<typename U>
  constexpr vec3& operator*=(U scalar)
  {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
  }

  constexpr vec3 operator*(vec3 other)
  {
    return { x * other.x, y * other.y, z * other.z };
  }

  // Division =====

  template<typename U>
  constexpr vec3 operator/(U scalar)
  {
    return { x / scalar, y / scalar, z / scalar };
  }

  template<typename U>
  constexpr vec3& operator/=(U scalar)
  {
    x /= scalar;
    y /= scalar;
    z /= scalar;
    return *this;
  }

  // Addition =====

  template<typename U>
  constexpr vec3 operator+(U scalar) const
  {
    return { x + scalar, y + scalar, z + scalar };
  }

  template<typename U>
  constexpr vec3& operator+=(U scalar)
  {
    x += scalar;
    y += scalar;
    z += scalar;
    return *this;
  }

  constexpr vec3 operator+(vec3 other) const
  {
    return { x + other.x, y + other.y, z + other.z };
  }

  constexpr vec3& operator+=(vec3 other)
  {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
  }

  // Subtraction =====

  template<typename U>
  constexpr vec3 operator-(U scalar)
  {
    return { x - scalar, y - scalar, z - scalar };
  }

  template<typename U>
  constexpr vec3& operator-=(U scalar)
  {
    x -= scalar;
    y -= scalar;
    z -= scalar;
    return *this;
  }

  constexpr vec3 operator-(vec3 other)
  {
    return { x - other.x, y - other.y, z - other.z };
  }

  constexpr vec3& operator-=(vec3 other)
  {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
  }

  //=========================
  // Operations
  //=========================

  //constexpr f32 Combined()
  //{
  //  return x + y + z + w;
  //}

  constexpr f32 Dot(vec3 other)
  {
    return x * other.x + y * other.y + z * other.z;
  }

  constexpr vec3 Normal()
  {
    if (x == 0.0f && y == 0.0f && z == 0.0f)
      return *this;

    return *this / (f32)sqrt(x * x + y * y + z * z);
  }

  constexpr vec3& Normalize()
  {
    if (x == 0.0f && y == 0.0f && z == 0.0f)
      return *this;

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

  constexpr vec4(f32 _x, f32 _y, f32 _z, f32 _w) : x(_x), y(_y), z(_z), w(_w) {}
  constexpr vec4(f32 _val) : x(_val), y(_val), z(_val), w(_val) {}
  constexpr vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}

  //=========================
  // Operators
  //=========================

  constexpr f32& operator[](int i)
  {
    assert(i < 4);

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
    assert(i < 4);

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

  constexpr bool operator==(vec4& other) const
  {
    return x == other.x && y == other.y && z == other.z && w == other.w;
  }

  constexpr bool operator==(const vec4& other) const
  {
    return x == other.x && y == other.y && z == other.z && w == other.w;
  }

  //=========================
  // Mathematic Operators
  //=========================

  // Multiplication =====

  template<typename U>
  constexpr vec4 operator*(U scalar)
  {
    return { x * scalar, y * scalar, z * scalar, w * scalar };
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

  constexpr vec4 operator*(vec4 other)
  {
    return { x * other.x, y * other.y, z * other.z, w * other.w };
  }

  // Division =====

  template<typename U>
  constexpr vec4 operator/(U scalar)
  {
    return { x / scalar, y / scalar, z / scalar, w / scalar };
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

  // Addition =====

  template<typename U>
  constexpr vec4 operator+(U scalar)
  {
    return { x + scalar, y + scalar, z + scalar, w + scalar };
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

  constexpr vec4 operator+(vec4 other)
  {
    return { x + other.x, y + other.y, z + other.z, w + other.w };
  }

  constexpr vec4& operator+=(vec4 other)
  {
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;
    return *this;
  }

  // Subtraction =====

  template<typename U>
  constexpr vec4 operator-(U scalar)
  {
    return { x - scalar, y - scalar, z - scalar, w - scalar };
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

  constexpr vec4 operator-(vec4 other)
  {
    return { x - other.x, y - other.y, z - other.z, w - other.w };
  }

  constexpr vec4& operator-=(vec4 other)
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

  constexpr f32 Dot(vec4 other)
  {
    return x * other.x + y * other.y + z * other.z + w * other.w;
  }

  constexpr vec4 Normal()
  {
    if (x == 0.0f && y == 0.0f && z == 0.0f && w == 0.0f)
      return *this;

    return *this / (f32)sqrt(x * x + y * y + z * z + w * w);
  }

  constexpr vec4& Normalize()
  {
    if (x == 0.0f && y == 0.0f && z == 0.0f && w == 0.0f)
      return *this;

    *this /= (f32)sqrt(x * x + y * y + z * z + w * w);
    return *this;
  }

} vec4;

//=========================
//=========================
// I32
//=========================
//=========================


typedef struct vec2I
{
  union { i32 x, r, width, pitch; };
  union { i32 y, g, height, yaw; };

  constexpr vec2I(i32 _x, i32 _y) : x(_x), y(_y) {}
  constexpr vec2I(i32 _val) : x(_val), y(_val) {}
  constexpr vec2I() : x(0), y(0) {}

  //=========================
  // Operators
  //=========================

  constexpr i32& operator[](int i)
  {
    assert(i < 2);

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
    assert(i < 2);

    switch (i)
    {
    default:
    case 0:
      return x;
    case 1:
      return y;
    }
  }

  constexpr bool operator==(vec2I& other) const
  {
    return x == other.x && y == other.y;
  }

  constexpr bool operator==(const vec2I& other) const
  {
    return x == other.x && y == other.y;
  }

  //=========================
  // Mathematic Operators
  //=========================

  // Multiplication =====

  template<typename U>
  constexpr vec2I operator*(U scalar)
  {
    return { x * scalar, y * scalar };
  }

  template<typename U>
  constexpr vec2I& operator*=(U scalar)
  {
    x *= scalar;
    y *= scalar;
    return *this;
  }

  constexpr vec2I operator*(vec2I other)
  {
    return { x * other.x, y * other.y };
  }

  // Division =====

  template<typename U>
  constexpr vec2I operator/(U scalar)
  {
    return { x / scalar, y / scalar };
  }

  template<typename U>
  constexpr vec2I& operator/=(U scalar)
  {
    x /= scalar;
    y /= scalar;
    return *this;
  }

  // Addition =====

  template<typename U>
  constexpr vec2I operator+(U scalar)
  {
    return { x + scalar, y + scalar };
  }

  template<typename U>
  constexpr vec2I& operator+=(U scalar)
  {
    x += scalar;
    y += scalar;
    return *this;
  }

  constexpr vec2I operator+(vec2I other)
  {
    return { x + other.x, y + other.y };
  }

  constexpr vec2I& operator+=(vec2I other)
  {
    x += other.x;
    y += other.y;
    return *this;
  }

  // Subtraction =====

  template<typename U>
  constexpr vec2I operator-(U scalar)
  {
    return { x - scalar, y - scalar };
  }

  template<typename U>
  constexpr vec2I& operator-=(U scalar)
  {
    x -= scalar;
    y -= scalar;
    return *this;
  }

  constexpr vec2I operator-(vec2I other)
  {
    return { x - other.x, y - other.y };
  }

  constexpr vec2I& operator-=(vec2I other)
  {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  //=========================
  // Operations
  //=========================

  //constexpr i32 Combined()
  //{
  //  return x + y + z + w;
  //}

  constexpr i32 Dot(vec2I other)
  {
    return x * other.x + y * other.y;
  }

  constexpr vec2I Normal()
  {
    if (x == 0 && y == 0)
      return *this;

    return *this / (i32)sqrt(x * x + y * y);
  }

  constexpr vec2I& Normalize()
  {
    if (x == 0 && y == 0)
      return *this;

    *this /= (i32)sqrt(x * x + y * y);
    return *this;
  }

} vec2I;

typedef struct vec3I
{
  union { i32 x, r; };
  union { i32 y, g; };
  union { i32 z, b; };

  constexpr vec3I(i32 _x, i32 _y, i32 _z) : x(_x), y(_y), z(_z) {}
  constexpr vec3I(i32 _val) : x(_val), y(_val), z(_val) {}
  constexpr vec3I() : x(0), y(0), z(0) {}

  //=========================
  // Operators
  //=========================

  constexpr i32& operator[](int i)
  {
    assert(i < 3);

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
    assert(i < 3);

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

  constexpr bool operator==(vec3I& other) const
  {
    return x == other.x && y == other.y && z == other.z;
  }

  constexpr bool operator==(const vec3I& other) const
  {
    return x == other.x && y == other.y && z == other.z;
  }

  //=========================
  // Mathematic Operators
  //=========================

  // Multiplication =====

  template<typename U>
  constexpr vec3I operator*(U scalar)
  {
    return { x * scalar, y * scalar, z * scalar };
  }

  template<typename U>
  constexpr vec3I& operator*=(U scalar)
  {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
  }

  constexpr vec3I operator*(vec3I other)
  {
    return { x * other.x, y * other.y, z * other.z };
  }

  // Division =====

  template<typename U>
  constexpr vec3I operator/(U scalar)
  {
    return { x / scalar, y / scalar, z / scalar };
  }

  template<typename U>
  constexpr vec3I& operator/=(U scalar)
  {
    x /= scalar;
    y /= scalar;
    z /= scalar;
    return *this;
  }

  // Addition =====

  template<typename U>
  constexpr vec3I operator+(U scalar)
  {
    return { x + scalar, y + scalar, z + scalar };
  }

  template<typename U>
  constexpr vec3I& operator+=(U scalar)
  {
    x += scalar;
    y += scalar;
    z += scalar;
    return *this;
  }

  constexpr vec3I operator+(vec3I other)
  {
    return { x + other.x, y + other.y, z + other.z };
  }

  constexpr vec3I& operator+=(vec3I other)
  {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
  }

  // Subtraction =====

  template<typename U>
  constexpr vec3I operator-(U scalar)
  {
    return { x - scalar, y - scalar, z - scalar };
  }

  template<typename U>
  constexpr vec3I& operator-=(U scalar)
  {
    x -= scalar;
    y -= scalar;
    z -= scalar;
    return *this;
  }

  constexpr vec3I operator-(vec3I other)
  {
    return { x - other.x, y - other.y, z - other.z };
  }

  constexpr vec3I& operator-=(vec3I other)
  {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
  }

  //=========================
  // Operations
  //=========================

  //constexpr i32 Combined()
  //{
  //  return x + y + z + w;
  //}

  constexpr i32 Dot(vec3I other)
  {
    return x * other.x + y * other.y + z * other.z;
  }

  constexpr vec3I Normal()
  {
    if (x == 0 && y == 0 && z == 0)
      return *this;

    return *this / (i32)sqrt(x * x + y * y + z * z);
  }

  constexpr vec3I& Normalize()
  {
    if (x == 0 && y == 0 && z == 0)
      return *this;

    *this /= (i32)sqrt(x * x + y * y + z * z);
    return *this;
  }

} vec3I;

typedef struct vec4I
{
  union { i32 x, r; };
  union { i32 y, g; };
  union { i32 z, b; };
  union { i32 w, a; };

  constexpr vec4I(i32 _x, i32 _y, i32 _z, i32 _w) : x(_x), y(_y), z(_z), w(_w) {}
  constexpr vec4I(i32 _val) : x(_val), y(_val), z(_val), w(_val) {}
  constexpr vec4I() : x(0), y(0), z(0), w(0) {}

  //=========================
  // Operators
  //=========================

  constexpr i32& operator[](int i)
  {
    assert(i < 4);

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
    assert(i < 4);

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

  constexpr bool operator==(vec4I& other) const
  {
    return x == other.x && y == other.y && z == other.z && w == other.w;
  }

  constexpr bool operator==(const vec4I& other) const
  {
    return x == other.x && y == other.y && z == other.z && w == other.w;
  }

  //=========================
  // Mathematic Operators
  //=========================

  // Multiplication =====

  template<typename U>
  constexpr vec4I operator*(U scalar)
  {
    return { x * scalar, y * scalar, z * scalar, w * scalar };
  }

  template<typename U>
  constexpr vec4I& operator*=(U scalar)
  {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;
    return *this;
  }

  constexpr vec4I operator*(vec4I other)
  {
    return { x * other.x, y * other.y, z * other.z, w * other.w };
  }

  // Division =====

  template<typename U>
  constexpr vec4I operator/(U scalar)
  {
    return { x / scalar, y / scalar, z / scalar, w / scalar };
  }

  template<typename U>
  constexpr vec4I& operator/=(U scalar)
  {
    x /= scalar;
    y /= scalar;
    z /= scalar;
    w /= scalar;
    return *this;
  }

  // Addition =====

  template<typename U>
  constexpr vec4I operator+(U scalar)
  {
    return { x + scalar, y + scalar, z + scalar, w + scalar };
  }

  template<typename U>
  constexpr vec4I& operator+=(U scalar)
  {
    x += scalar;
    y += scalar;
    z += scalar;
    w += scalar;
    return *this;
  }

  constexpr vec4I operator+(vec4I other)
  {
    return { x + other.x, y + other.y, z + other.z, w + other.w };
  }

  constexpr vec4I& operator+=(vec4I other)
  {
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;
    return *this;
  }

  // Subtraction =====

  template<typename U>
  constexpr vec4I operator-(U scalar)
  {
    return { x - scalar, y - scalar, z - scalar, w - scalar };
  }

  template<typename U>
  constexpr vec4I& operator-=(U scalar)
  {
    x -= scalar;
    y -= scalar;
    z -= scalar;
    w -= scalar;
    return *this;
  }

  constexpr vec4I operator-(vec4I other)
  {
    return { x - other.x, y - other.y, z - other.z, w - other.w };
  }

  constexpr vec4I& operator-=(vec4I other)
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

  //constexpr i32 Combined()
  //{
  //  return x + y + z + w;
  //}

  constexpr i32 Dot(vec4I other)
  {
    return x * other.x + y * other.y + z * other.z + w * other.w;
  }

  constexpr vec4I Normal()
  {
    if (x == 0 && y == 0 && z == 0 && w == 0)
      return *this;

    return *this / (i32)sqrt(x * x + y * y + z * z + w * w);
  }

  constexpr vec4I& Normalize()
  {
    if (x == 0 && y == 0 && z == 0 && w == 0)
      return *this;

    *this /= (i32)sqrt(x * x + y * y + z * z + w * w);
    return *this;
  }

} vec4I;

//=========================
//=========================
// U32
//=========================
//=========================


typedef struct vec2U
{
  union { u32 x, r, width, pitch; };
  union { u32 y, g, height, yaw; };

  constexpr vec2U(u32 _x, u32 _y) : x(_x), y(_y) {}
  constexpr vec2U(u32 _val) : x(_val), y(_val) {}
  constexpr vec2U() : x(0u), y(0u) {}

  //=========================
  // Operators
  //=========================

  constexpr u32& operator[](int i)
  {
    assert(i < 2);

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
    assert(i < 2);

    switch (i)
    {
    default:
    case 0:
      return x;
    case 1:
      return y;
    }
  }

  constexpr bool operator==(vec2U& other) const
  {
    return x == other.x && y == other.y;
  }

  constexpr bool operator==(const vec2U& other) const
  {
    return x == other.x && y == other.y;
  }

  //=========================
  // Mathematic Operators
  //=========================

  // Multiplication =====

  template<typename U>
  constexpr vec2U operator*(U scalar)
  {
    return { x * scalar, y * scalar };
  }

  template<typename U>
  constexpr vec2U& operator*=(U scalar)
  {
    x *= scalar;
    y *= scalar;
    return *this;
  }

  constexpr vec2U operator*(vec2U other)
  {
    return { x * other.x, y * other.y };
  }

  // Division =====

  template<typename U>
  constexpr vec2U operator/(U scalar)
  {
    return { x / scalar, y / scalar };
  }

  template<typename U>
  constexpr vec2U& operator/=(U scalar)
  {
    x /= scalar;
    y /= scalar;
    return *this;
  }

  // Addition =====

  template<typename U>
  constexpr vec2U operator+(U scalar)
  {
    return { x + scalar, y + scalar };
  }

  template<typename U>
  constexpr vec2U& operator+=(U scalar)
  {
    x += scalar;
    y += scalar;
    return *this;
  }

  constexpr vec2U operator+(vec2U other)
  {
    return { x + other.x, y + other.y };
  }

  constexpr vec2U& operator+=(vec2U other)
  {
    x += other.x;
    y += other.y;
    return *this;
  }

  // Subtraction =====

  template<typename U>
  constexpr vec2U operator-(U scalar)
  {
    return { x - scalar, y - scalar };
  }

  template<typename U>
  constexpr vec2U& operator-=(U scalar)
  {
    x -= scalar;
    y -= scalar;
    return *this;
  }

  constexpr vec2U operator-(vec2U other)
  {
    return { x - other.x, y - other.y };
  }

  constexpr vec2U& operator-=(vec2U other)
  {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  //=========================
  // Operations
  //=========================

  //constexpr u32 Combined()
  //{
  //  return x + y + z + w;
  //}

  constexpr u32 Dot(vec2U other)
  {
    return x * other.x + y * other.y;
  }

  constexpr vec2U Normal()
  {
    if (x == 0u && y == 0u)
      return *this;

    return *this / (u32)sqrt(x * x + y * y);
  }

  constexpr vec2U& Normalize()
  {
    if (x == 0u && y == 0u)
      return *this;

    *this /= (u32)sqrt(x * x + y * y);
    return *this;
  }

} vec2U;

typedef struct vec3U
{
  union { u32 x, r; };
  union { u32 y, g; };
  union { u32 z, b; };

  constexpr vec3U(u32 _x, u32 _y, u32 _z) : x(_x), y(_y), z(_z) {}
  constexpr vec3U(u32 _val) : x(_val), y(_val), z(_val) {}
  constexpr vec3U() : x(0u), y(0u), z(0u) {}

  //=========================
  // Operators
  //=========================

  constexpr u32& operator[](int i)
  {
    assert(i < 3);

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
    assert(i < 3);

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

  constexpr bool operator==(vec3U& other) const
  {
    return x == other.x && y == other.y && z == other.z;
  }

  constexpr bool operator==(const vec3U& other) const
  {
    return x == other.x && y == other.y && z == other.z;
  }

  //=========================
  // Mathematic Operators
  //=========================

  // Multiplication =====

  template<typename U>
  constexpr vec3U operator*(U scalar)
  {
    return { x * scalar, y * scalar, z * scalar };
  }

  template<typename U>
  constexpr vec3U& operator*=(U scalar)
  {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
  }

  constexpr vec3U operator*(vec3U other)
  {
    return { x * other.x, y * other.y, z * other.z };
  }

  // Division =====

  template<typename U>
  constexpr vec3U operator/(U scalar)
  {
    return { x / scalar, y / scalar, z / scalar };
  }

  template<typename U>
  constexpr vec3U& operator/=(U scalar)
  {
    x /= scalar;
    y /= scalar;
    z /= scalar;
    return *this;
  }

  // Addition =====

  template<typename U>
  constexpr vec3U operator+(U scalar)
  {
    return { x + scalar, y + scalar, z + scalar };
  }

  template<typename U>
  constexpr vec3U& operator+=(U scalar)
  {
    x += scalar;
    y += scalar;
    z += scalar;
    return *this;
  }

  constexpr vec3U operator+(vec3U other)
  {
    return { x + other.x, y + other.y, z + other.z };
  }

  constexpr vec3U& operator+=(vec3U other)
  {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
  }

  // Subtraction =====

  template<typename U>
  constexpr vec3U operator-(U scalar)
  {
    return { x - scalar, y - scalar, z - scalar };
  }

  template<typename U>
  constexpr vec3U& operator-=(U scalar)
  {
    x -= scalar;
    y -= scalar;
    z -= scalar;
    return *this;
  }

  constexpr vec3U operator-(vec3U other)
  {
    return { x - other.x, y - other.y, z - other.z };
  }

  constexpr vec3U& operator-=(vec3U other)
  {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
  }

  //=========================
  // Operations
  //=========================

  //constexpr u32 Combined()
  //{
  //  return x + y + z + w;
  //}

  constexpr u32 Dot(vec3U other)
  {
    return x * other.x + y * other.y + z * other.z;
  }

  constexpr vec3U Normal()
  {
    if (x == 0u && y == 0u && z == 0u)
      return *this;

    return *this / (u32)sqrt(x * x + y * y + z * z);
  }

  constexpr vec3U& Normalize()
  {
    if (x == 0u && y == 0u && z == 0u)
      return *this;

    *this /= (u32)sqrt(x * x + y * y + z * z);
    return *this;
  }

} vec3U;

typedef struct vec4U
{
  union { u32 x, r; };
  union { u32 y, g; };
  union { u32 z, b; };
  union { u32 w, a; };

  constexpr vec4U(u32 _x, u32 _y, u32 _z, u32 _w) : x(_x), y(_y), z(_z), w(_w) {}
  constexpr vec4U(u32 _val) : x(_val), y(_val), z(_val), w(_val) {}
  constexpr vec4U() : x(0u), y(0u), z(0u), w(0u) {}

  //=========================
  // Operators
  //=========================

  constexpr u32& operator[](int i)
  {
    assert(i < 4);

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
    assert(i < 4);

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

  constexpr bool operator==(vec4U& other) const
  {
    return x == other.x && y == other.y && z == other.z && w == other.w;
  }

  constexpr bool operator==(const vec4U& other) const
  {
    return x == other.x && y == other.y && z == other.z && w == other.w;
  }

  //=========================
  // Mathematic Operators
  //=========================

  // Multiplication =====

  template<typename U>
  constexpr vec4U operator*(U scalar)
  {
    return { x * scalar, y * scalar, z * scalar, w * scalar };
  }

  template<typename U>
  constexpr vec4U& operator*=(U scalar)
  {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;
    return *this;
  }

  constexpr vec4U operator*(vec4U other)
  {
    return { x * other.x, y * other.y, z * other.z, w * other.w };
  }

  // Division =====

  template<typename U>
  constexpr vec4U operator/(U scalar)
  {
    return { x / scalar, y / scalar, z / scalar, w / scalar };
  }

  template<typename U>
  constexpr vec4U& operator/=(U scalar)
  {
    x /= scalar;
    y /= scalar;
    z /= scalar;
    w /= scalar;
    return *this;
  }

  // Addition =====

  template<typename U>
  constexpr vec4U operator+(U scalar)
  {
    return { x + scalar, y + scalar, z + scalar, w + scalar };
  }

  template<typename U>
  constexpr vec4U& operator+=(U scalar)
  {
    x += scalar;
    y += scalar;
    z += scalar;
    w += scalar;
    return *this;
  }

  constexpr vec4U operator+(vec4U other)
  {
    return { x + other.x, y + other.y, z + other.z, w + other.w };
  }

  constexpr vec4U& operator+=(vec4U other)
  {
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;
    return *this;
  }

  // Subtraction =====

  template<typename U>
  constexpr vec4U operator-(U scalar)
  {
    return { x - scalar, y - scalar, z - scalar, w - scalar };
  }

  template<typename U>
  constexpr vec4U& operator-=(U scalar)
  {
    x -= scalar;
    y -= scalar;
    z -= scalar;
    w -= scalar;
    return *this;
  }

  constexpr vec4U operator-(vec4U other)
  {
    return { x - other.x, y - other.y, z - other.z, w - other.w };
  }

  constexpr vec4U& operator-=(vec4U other)
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

  //constexpr u32 Combined()
  //{
  //  return x + y + z + w;
  //}

  constexpr u32 Dot(vec4U other)
  {
    return x * other.x + y * other.y + z * other.z + w * other.w;
  }

  constexpr vec4U Normal()
  {
    if (x == 0u && y == 0u && z == 0u && w == 0u)
      return *this;

    return *this / (u32)sqrt(x * x + y * y + z * z + w * w);
  }

  constexpr vec4U& Normalize()
  {
    if (x == 0u && y == 0u && z == 0u && w == 0u)
      return *this;

    *this /= (u32)sqrt(x * x + y * y + z * z + w * w);
    return *this;
  }

} vec4U;

} // namespace Ice

#endif // !define ICE_MATH_VECTOR_H_
