
#ifndef ICE_MATH_LINEAR_H_
#define ICE_MATH_LINEAR_H_

#include "defines.h"

#include "math/vector.h"
#include "math/matrix.h"

float DotV4V4(vec4 _l, vec4 _r)
{
  return (_l.x * _r.x) + (_l.y * _r.y) + (_l.z * _r.z) + (_l.w * _r.w);
}

vec3 Cross(vec3 a, vec3 b)
{
  return { (a.y * b.z) - (a.z * b.y), (a.z * b.x) - (a.x * b.z), (a.x * b.y) - (a.y * b.x) };
}

#endif // !define ICE_MATH_VECTOR_H_
