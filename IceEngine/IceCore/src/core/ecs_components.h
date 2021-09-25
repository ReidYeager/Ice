
#ifndef ICE_CORE_ECS_COMPONENTS_H_
#define ICE_CORE_ECS_COMPONENTS_H_

#include "defines.h"

struct TransformComponent
{
  // Position only
  f32 x;
  f32 y;
  f32 z;
};

struct RenderableComponent
{
  // Mesh
  u32 meshIndex;
  u32 materialIndex;
};

#endif // !define ICE_CORE_COMPONENTS_H_
