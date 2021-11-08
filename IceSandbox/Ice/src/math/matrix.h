
#ifndef ICE_MATH_MATRIX_H_
#define ICE_MATH_MATRIX_H_

#include "asserts.h"
#include "defines.h"

#include "math/vector.h"

typedef union mat4
{
  struct
  {
    vec4 x;
    vec4 y;
    vec4 z;
    vec4 w;
  };
  struct
  {
    vec4 r;
    vec4 g;
    vec4 b;
    vec4 a;
  };
  struct
  {
    vec4 row0;
    vec4 row1;
    vec4 row2;
    vec4 row3;
  };

  constexpr vec4& operator[](int i)
  {
    ICE_ASSERT(i < 4);
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

  constexpr vec4 const& operator[](int i) const
  {
    ICE_ASSERT(i < 4);
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
} mat4;

#endif // !define ICE_MATH_MATRIX_H_
