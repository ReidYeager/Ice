
#ifndef ICE_MATH_MATRIX_H_
#define ICE_MATH_MATRIX_H_

#include "defines.h"

#include "math/vector.hpp"

namespace Ice {

// Column-major matrix
typedef union mat4
{
  struct
  {
    Ice::vec4 x;
    Ice::vec4 y;
    Ice::vec4 z;
    Ice::vec4 w;
  };
  struct
  {
    Ice::vec4 r;
    Ice::vec4 g;
    Ice::vec4 b;
    Ice::vec4 a;
  };
  struct
  {
    Ice::vec4 col0;
    Ice::vec4 col1;
    Ice::vec4 col2;
    Ice::vec4 col3;
  };

  //=========================
  // Operators
  //=========================

  // Reads array column-by-column
  constexpr mat4 const& operator=(f32* in)
  {
    for (u32 col = 0; col < 4; col++)
    {
      for (u32 row = 0; row < 4; row++)
      {
        switch (col)
        {
        case 0:
          col0[row] = in[(4 * col) + row];
        case 1:
          col1[row] = in[(4 * col) + row];
        case 2:
          col2[row] = in[(4 * col) + row];
        case 3:
          col3[row] = in[(4 * col) + row];
        }
      }
    }

    return *this;
  }

  constexpr Ice::vec4& operator[](int i)
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

  constexpr Ice::vec4 const& operator[](int i) const
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

  //=========================
  // Mathematic Operators
  //=========================

  // Multiplication =====

  // For rotation-only transforms
  constexpr Ice::vec3 operator*(Ice::vec3 _vector)
  {
    return {
      x.x * _vector.x + y.x * _vector.y + z.x * _vector.z,
      x.y * _vector.x + y.y * _vector.y + z.y * _vector.z,
      x.z * _vector.x + y.z * _vector.y + z.z * _vector.z,
    };
  }

  constexpr Ice::vec4 operator*(Ice::vec4 _vector)
  {
    return {
      x.Dot(_vector),
      y.Dot(_vector),
      z.Dot(_vector),
      w.Dot(_vector)
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

  //=========================
  // Operations
  //=========================

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
                            0.0f, 0.0f, 0.0f, 1.0f };

} // namespace Ice

#endif // !define ICE_MATH_MATRIX_H_
