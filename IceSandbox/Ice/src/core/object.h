
#ifndef ICE_CORE_OBJECT_H_
#define ICE_CORE_OBJECT_H_

#include "defines.h"

#include "math/vector.h"

#include "rendering/vulkan/vk_context.h"

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <vector>

struct IceTransform
{
  glm::vec3 position = { 0.0f, 0.0f, 0.0f };
  glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
  glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

  IceTransform* parent;

  glm::mat4 matrix;
  IvkBuffer buffer; // Tmp fix for buffers moving around in renderer

  glm::mat4 UpdateMatrix();
};

struct IceObject
{
  IceTransform transform;
  u32 meshHandle;
  u32 materialHandle;

  std::vector<IceObject*> children;
};

#endif // !define ICE_CORE_OBJECT_H_
