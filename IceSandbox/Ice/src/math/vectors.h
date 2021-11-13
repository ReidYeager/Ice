
#ifndef ICE_MATH_VEC2_H_
#define ICE_MATH_VEC2_H_

#include "defines.h"

// =======================
// Vec2s
// =======================

typedef union vec2_union
{
  struct
  {
    union { f32 x, r, u, width; };
    union { f32 y, g, v, height; };
  };
  f32 elements[2];
} vec2;

typedef union vec2Int_union
{
  struct
  {
    union { i32 x, r, u, width; };
    union { i32 y, g, v, height; };
  };
  i32 elements[2];
} vec2I;

typedef union vec2UInt_union
{
  struct
  {
    union { u32 x, r, u, width; };
    union { u32 y, g, v, height; };
  };
  u32 elements[2];
} vec2U;

// =======================
// Vec3s
// =======================

typedef union vec3_union
{
  struct
  {
    union { f32 x, r, u; };
    union { f32 y, g, v; };
    union { f32 z, b, t; };
  };
  f32 elements[3];
} vec3;

typedef union vec3Int_union
{
  struct
  {
    union { i32 x, r, u; };
    union { i32 y, g, v; };
    union { i32 z, b, t; };
  };
  i32 elements[3];
} vec3I;

typedef union vec3UInt_union
{
  struct
  {
    union { u32 x, r, u; };
    union { u32 y, g, v; };
    union { u32 z, b, t; };
  };
  u32 elements[3];
} vecU3;

// =======================
// Vec4s
// =======================

typedef union vec4_union
{
  struct
  {
    union { f32 x, r, u; };
    union { f32 y, g, v; };
    union { f32 z, b, t; };
    union { f32 w, a/*w*/; };
  };
  f32 elements[4];
} vec4;

typedef union vec4Int_union
{
  struct
  {
    union { i32 x, r, u; };
    union { i32 y, g, v; };
    union { i32 z, b, t; };
    union { i32 w, a/*w*/; };
  };
  i32 elements[4];
} vec4I;

typedef union vec4UInt_union
{
  struct
  {
    union { u32 x, r, u; };
    union { u32 y, g, v; };
    union { u32 z, b, t; };
    union { u32 w, a/*w*/; };
  };
  u32 elements[4];
} vec4U;

// =======================
// Other
// =======================

typedef vec4 quaternion;

typedef union mat4_union
{
  f32 elements[16];
  vec4 rows[4];
  struct
  {
    vec4 x;
    vec4 y;
    vec4 z;
    vec4 w;
  };
} mat4;

#endif // !define ICE_MATH_VEC2_H_
