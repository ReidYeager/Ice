
#ifndef ICE_CORE_ECS_COMPONENTS_H_
#define ICE_CORE_ECS_COMPONENTS_H_

#include "defines.h"

#include "math/vector.h"

struct TransformComponent
{
  vec3 position;
  vec3 rotation;
  vec3 scale;
};

// Required to render a mesh
struct RenderableComponent
{
  u32 meshIndex;
  u32 materialIndex;
};

#endif // !define ICE_CORE_COMPONENTS_H_
