
#ifndef ICE_CORE_CAMERA_H_
#define ICE_CORE_CAMERA_H_

#include "math/vector.h"

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

struct IceCamera
{
  // Need to replace with a Transform

  glm::mat4 viewProjectionMatrix;
};

#endif // !define ICE_CORE_CAMERA_H_
