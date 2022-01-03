
#ifndef ICE_CORE_OBJECT_H_
#define ICE_CORE_OBJECT_H_

#include "defines.h"

#include "math/vector.h"

#include <vector>

struct IceTransform
{
  vec3 position = { 0.0f, 0.0f, 0.0f };
  vec3 rotation = { 0.0f, 0.0f, 0.0f };
  vec3 scale = { 1.0f, 1.0f, 1.0f };

  IceTransform* parent;
};

struct IceObject
{
  IceTransform transform;
  u32 meshHandle;
  u32 materialHandle;

  std::vector<IceObject*> children;
};

#endif // !define ICE_CORE_OBJECT_H_
