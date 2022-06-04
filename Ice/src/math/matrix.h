
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

  // For rotation-only transforms
  constexpr vec3 operator*(vec3 vector)
  {
    return {
      x.x * vector.x + x.y * vector.y + x.z * vector.z,
      y.x * vector.x + y.y * vector.y + y.z * vector.z,
      z.x * vector.x + z.y * vector.y + z.z * vector.z,
    };
  }

  constexpr vec4 operator*(vec4 vector)
  {
    return {
      x.Dot(vector),
      y.Dot(vector),
      z.Dot(vector),
      w.Dot(vector)
    };
  }

  constexpr mat4 operator*(mat4 mat)
  {
    mat = mat.Transpose();

    return {
      x.Dot(mat.x), x.Dot(mat.y), x.Dot(mat.z), x.Dot(mat.w),
      y.Dot(mat.x), y.Dot(mat.y), y.Dot(mat.z), y.Dot(mat.w),
      z.Dot(mat.x), z.Dot(mat.y), z.Dot(mat.z), z.Dot(mat.w),
      w.Dot(mat.x), w.Dot(mat.y), w.Dot(mat.z), w.Dot(mat.w)
    };
  }

  constexpr mat4 Transpose() const
  {
    return { x.x, y.x, z.x, w.x,
             x.y, y.y, z.y, w.y,
             x.z, y.z, z.z, w.z,
             x.w, y.w, z.w, w.w };
  }

} mat4;

const mat4 mat4Identity = { 1.0f, 0.0f, 0.0f, 0.0f,
                            0.0f, 1.0f, 0.0f, 0.0f,
                            0.0f, 0.0f, 1.0f, 0.0f,
                            0.0f, 0.0f, 0.0f, 1.0f};

#endif // !define ICE_MATH_MATRIX_H_
