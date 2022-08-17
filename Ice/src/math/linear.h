
#ifndef ICE_MATH_LINEAR_H_
#define ICE_MATH_LINEAR_H_

#include "math/vector.hpp"
#include "math/quaternion.hpp"
#include "math/matrix.hpp"

namespace Ice {

template<typename U>
U Clamp(U _value, U _min, U _max)
{
  if (_value > _max)
    return _max;
  else if (_value < _min)
    return _min;

  return _value;
}

template<typename U>
U Wrap(U _value, U _min, U _max)
{
  if (_value > _max)
    return (_value - _max) + _min;
  else if (_value < _min)
    return (_value - _min) + _max;

  return _value;
}

} // namespace Ice

#endif // !define ICE_MATH_VECTOR_H_
