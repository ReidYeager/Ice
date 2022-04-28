
#ifndef ICE_MATH_MATRIX_H_
#define ICE_MATH_MATRIX_H_

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

  constexpr mat4 Transpose() const
  {
    return { x.x, y.x, z.x, w.x,
             x.y, y.y, z.y, w.y,
             x.z, y.z, z.z, w.z,
             x.w, y.w, z.w, w.w };
  }

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

  constexpr mat4 const& operator=(f32* in)
  {
    for (u32 i = 0; i < 4; i++)
    {
      for (u32 j = 0; j < 4; j++)
      {
        switch (i)
        {
        default:
        case 0:
          x[j] = *(in + (i * 4) + j);
          break;
        case 1:
          y[j] = *(in + (i * 4) + j);
          break;
        case 2:
          z[j] = *(in + (i * 4) + j);
          break;
        case 3:
          w[j] = *(in + (i * 4) + j);
          break;
        }
      }
    }

    return *this;
  }

  constexpr vec4 operator*(vec4 vector)
  {
    return {
      x * vector,
      y * vector,
      z * vector,
      w * vector
    };
  }

  constexpr mat4 operator*(mat4 mat)
  {
    mat = mat.Transpose();

    return {
      x * mat.x, x * mat.y, x * mat.z, x * mat.w,
      y * mat.x, y * mat.y, y * mat.z, y * mat.w,
      z * mat.x, z * mat.y, z * mat.z, z * mat.w,
      w * mat.x, w * mat.y, w * mat.z, w * mat.w
    };
  }
} mat4;

const mat4 mat4Identity = { 1.0f, 0.0f, 0.0f, 0.0f,
                            0.0f, 1.0f, 0.0f, 0.0f,
                            0.0f, 0.0f, 1.0f, 0.0f,
                            0.0f, 0.0f, 0.0f, 1.0f};

#endif // !define ICE_MATH_MATRIX_H_
