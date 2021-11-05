
#ifndef ICE_CORE_ECS_COMPONENTS_H_
#define ICE_CORE_ECS_COMPONENTS_H_

#include "defines.h"

struct TransformComponent
{
  // Position only
  float position[3];
  float rotation[3];
  float scale[3];
};

// Required to render a mesh
struct RenderableComponent
{
  u32 meshIndex;
  u32 materialIndex;
};

#endif // !define ICE_CORE_COMPONENTS_H_
