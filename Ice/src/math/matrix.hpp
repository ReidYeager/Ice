
#ifndef ICE_MATH_MATRIX_H_
#define ICE_MATH_MATRIX_H_

#include "defines.h"

#include "math/vector.hpp"

namespace Ice {

// Column-major matrix
typedef union mat4
{
  union
  {
    f32 elements[16];

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
  };

  mat4(f32* _elementArray)
  {
    elements[0 ] = _elementArray[0 ];
    elements[1 ] = _elementArray[1 ];
    elements[2 ] = _elementArray[2 ];
    elements[3 ] = _elementArray[3 ];
    elements[4 ] = _elementArray[4 ];
    elements[5 ] = _elementArray[5 ];
    elements[6 ] = _elementArray[6 ];
    elements[7 ] = _elementArray[7 ];
    elements[8 ] = _elementArray[8 ];
    elements[9 ] = _elementArray[9 ];
    elements[10] = _elementArray[10];
    elements[11] = _elementArray[11];
    elements[12] = _elementArray[12];
    elements[13] = _elementArray[13];
    elements[14] = _elementArray[14];
    elements[15] = _elementArray[15];
  }

  mat4(f32 _elementArray0 , f32 _elementArray1 , f32 _elementArray2 , f32 _elementArray3 ,
       f32 _elementArray4 , f32 _elementArray5 , f32 _elementArray6 , f32 _elementArray7 ,
       f32 _elementArray8 , f32 _elementArray9 , f32 _elementArray10, f32 _elementArray11,
       f32 _elementArray12, f32 _elementArray13, f32 _elementArray14, f32 _elementArray15)
  {
    elements[0 ] = _elementArray0;
    elements[1 ] = _elementArray1;
    elements[2 ] = _elementArray2;
    elements[3 ] = _elementArray3;
    elements[4 ] = _elementArray4;
    elements[5 ] = _elementArray5;
    elements[6 ] = _elementArray6;
    elements[7 ] = _elementArray7;
    elements[8 ] = _elementArray8;
    elements[9 ] = _elementArray9;
    elements[10] = _elementArray10;
    elements[11] = _elementArray11;
    elements[12] = _elementArray12;
    elements[13] = _elementArray13;
    elements[14] = _elementArray14;
    elements[15] = _elementArray15;
  }

  mat4(f32 _value = 1.0f)
  {
    elements[0 ] = _value;
    elements[1 ] = 0.0f;
    elements[2 ] = 0.0f;
    elements[3 ] = 0.0f;
    elements[4 ] = 0.0f;
    elements[5 ] = _value;
    elements[6 ] = 0.0f;
    elements[7 ] = 0.0f;
    elements[8 ] = 0.0f;
    elements[9 ] = 0.0f;
    elements[10] = _value;
    elements[11] = 0.0f;
    elements[12] = 0.0f;
    elements[13] = 0.0f;
    elements[14] = 0.0f;
    elements[15] = _value;
  }

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

  mat4 operator*(mat4 mat)
  {
    mat = mat.Transpose();

    return mat4(x.Dot(mat.x), x.Dot(mat.y), x.Dot(mat.z), x.Dot(mat.w),
                y.Dot(mat.x), y.Dot(mat.y), y.Dot(mat.z), y.Dot(mat.w),
                z.Dot(mat.x), z.Dot(mat.y), z.Dot(mat.z), z.Dot(mat.w),
                w.Dot(mat.x), w.Dot(mat.y), w.Dot(mat.z), w.Dot(mat.w));
  }

  //=========================
  // Operations
  //=========================

  mat4 Transpose() const
  {
    return mat4(x.x, y.x, z.x, w.x,
                x.y, y.y, z.y, w.y,
                x.z, y.z, z.z, w.z,
                x.w, y.w, z.w, w.w);
  }

  // TODO ~!!~ Invert matrix
  mat4 Inverse() const
  {
    Ice::mat4 inv;
    const Ice::mat4& m = *this;
    double det;

    inv.elements[0] = m.elements[5]  * m.elements[10] * m.elements[15] - 
                      m.elements[5]  * m.elements[11] * m.elements[14] - 
                      m.elements[9]  * m.elements[6]  * m.elements[15] + 
                      m.elements[9]  * m.elements[7]  * m.elements[14] +
                      m.elements[13] * m.elements[6]  * m.elements[11] - 
                      m.elements[13] * m.elements[7]  * m.elements[10];

    inv.elements[4] = -m.elements[4]  * m.elements[10] * m.elements[15] + 
                       m.elements[4]  * m.elements[11] * m.elements[14] + 
                       m.elements[8]  * m.elements[6]  * m.elements[15] - 
                       m.elements[8]  * m.elements[7]  * m.elements[14] - 
                       m.elements[12] * m.elements[6]  * m.elements[11] + 
                       m.elements[12] * m.elements[7]  * m.elements[10];

    inv.elements[8] = m.elements[4]  * m.elements[9] * m.elements[15] - 
                      m.elements[4]  * m.elements[11] * m.elements[13] - 
                      m.elements[8]  * m.elements[5] * m.elements[15] + 
                      m.elements[8]  * m.elements[7] * m.elements[13] + 
                      m.elements[12] * m.elements[5] * m.elements[11] - 
                      m.elements[12] * m.elements[7] * m.elements[9];

    inv.elements[12] = -m.elements[4]  * m.elements[9] * m.elements[14] + 
                        m.elements[4]  * m.elements[10] * m.elements[13] +
                        m.elements[8]  * m.elements[5] * m.elements[14] - 
                        m.elements[8]  * m.elements[6] * m.elements[13] - 
                        m.elements[12] * m.elements[5] * m.elements[10] + 
                        m.elements[12] * m.elements[6] * m.elements[9];

    inv.elements[1] = -m.elements[1]  * m.elements[10] * m.elements[15] + 
                       m.elements[1]  * m.elements[11] * m.elements[14] + 
                       m.elements[9]  * m.elements[2] * m.elements[15] - 
                       m.elements[9]  * m.elements[3] * m.elements[14] - 
                       m.elements[13] * m.elements[2] * m.elements[11] + 
                       m.elements[13] * m.elements[3] * m.elements[10];

    inv.elements[5] = m.elements[0]  * m.elements[10] * m.elements[15] - 
                      m.elements[0]  * m.elements[11] * m.elements[14] - 
                      m.elements[8]  * m.elements[2] * m.elements[15] + 
                      m.elements[8]  * m.elements[3] * m.elements[14] + 
                      m.elements[12] * m.elements[2] * m.elements[11] - 
                      m.elements[12] * m.elements[3] * m.elements[10];

    inv.elements[9] = -m.elements[0]  * m.elements[9] * m.elements[15] + 
                       m.elements[0]  * m.elements[11] * m.elements[13] + 
                       m.elements[8]  * m.elements[1] * m.elements[15] - 
                       m.elements[8]  * m.elements[3] * m.elements[13] - 
                       m.elements[12] * m.elements[1] * m.elements[11] + 
                       m.elements[12] * m.elements[3] * m.elements[9];

    inv.elements[13] = m.elements[0]  * m.elements[9] * m.elements[14] - 
                       m.elements[0]  * m.elements[10] * m.elements[13] - 
                       m.elements[8]  * m.elements[1] * m.elements[14] + 
                       m.elements[8]  * m.elements[2] * m.elements[13] + 
                       m.elements[12] * m.elements[1] * m.elements[10] - 
                       m.elements[12] * m.elements[2] * m.elements[9];

    inv.elements[2] = m.elements[1]  * m.elements[6] * m.elements[15] - 
                      m.elements[1]  * m.elements[7] * m.elements[14] - 
                      m.elements[5]  * m.elements[2] * m.elements[15] + 
                      m.elements[5]  * m.elements[3] * m.elements[14] + 
                      m.elements[13] * m.elements[2] * m.elements[7] - 
                      m.elements[13] * m.elements[3] * m.elements[6];

    inv.elements[6] = -m.elements[0]  * m.elements[6] * m.elements[15] + 
                       m.elements[0]  * m.elements[7] * m.elements[14] + 
                       m.elements[4]  * m.elements[2] * m.elements[15] - 
                       m.elements[4]  * m.elements[3] * m.elements[14] - 
                       m.elements[12] * m.elements[2] * m.elements[7] + 
                       m.elements[12] * m.elements[3] * m.elements[6];

    inv.elements[10] = m.elements[0]  * m.elements[5] * m.elements[15] - 
                       m.elements[0]  * m.elements[7] * m.elements[13] - 
                       m.elements[4]  * m.elements[1] * m.elements[15] + 
                       m.elements[4]  * m.elements[3] * m.elements[13] + 
                       m.elements[12] * m.elements[1] * m.elements[7] - 
                       m.elements[12] * m.elements[3] * m.elements[5];

    inv.elements[14] = -m.elements[0]  * m.elements[5] * m.elements[14] + 
                        m.elements[0]  * m.elements[6] * m.elements[13] + 
                        m.elements[4]  * m.elements[1] * m.elements[14] - 
                        m.elements[4]  * m.elements[2] * m.elements[13] - 
                        m.elements[12] * m.elements[1] * m.elements[6] + 
                        m.elements[12] * m.elements[2] * m.elements[5];

    inv.elements[3] = -m.elements[1] * m.elements[6] * m.elements[11] + 
                       m.elements[1] * m.elements[7] * m.elements[10] + 
                       m.elements[5] * m.elements[2] * m.elements[11] - 
                       m.elements[5] * m.elements[3] * m.elements[10] - 
                       m.elements[9] * m.elements[2] * m.elements[7] + 
                       m.elements[9] * m.elements[3] * m.elements[6];

    inv.elements[7] = m.elements[0] * m.elements[6] * m.elements[11] - 
                      m.elements[0] * m.elements[7] * m.elements[10] - 
                      m.elements[4] * m.elements[2] * m.elements[11] + 
                      m.elements[4] * m.elements[3] * m.elements[10] + 
                      m.elements[8] * m.elements[2] * m.elements[7] - 
                      m.elements[8] * m.elements[3] * m.elements[6];

    inv.elements[11] = -m.elements[0] * m.elements[5] * m.elements[11] + 
                        m.elements[0] * m.elements[7] * m.elements[9] + 
                        m.elements[4] * m.elements[1] * m.elements[11] - 
                        m.elements[4] * m.elements[3] * m.elements[9] - 
                        m.elements[8] * m.elements[1] * m.elements[7] + 
                        m.elements[8] * m.elements[3] * m.elements[5];

    inv.elements[15] = m.elements[0] * m.elements[5] * m.elements[10] - 
                       m.elements[0] * m.elements[6] * m.elements[9] - 
                       m.elements[4] * m.elements[1] * m.elements[10] + 
                       m.elements[4] * m.elements[2] * m.elements[9] + 
                       m.elements[8] * m.elements[1] * m.elements[6] - 
                       m.elements[8] * m.elements[2] * m.elements[5];

    det = m.elements[0] * inv.elements[0] +
          m.elements[1] * inv.elements[4] +
          m.elements[2] * inv.elements[8] +
          m.elements[3] * inv.elements[12];

    if (det == 0)
        return *this;

    det = 1.0 / det;

    return inv;
  }

} mat4;

const Ice::mat4 mat4Identity = Ice::mat4(1.0f);

} // namespace Ice

#endif // !define ICE_MATH_MATRIX_H_
