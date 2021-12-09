
#ifndef ICE_RENDERING_RE_RENDERER_BACKEND_H_
#define ICE_RENDERING_RE_RENDERER_BACKEND_H_

#include "defines.h"

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

struct IceCamera
{
  glm::mat4 viewProjectionMatrix;
};

class reIceRendererBackend
{
public:
  virtual b8 Initialize() = 0;
  virtual b8 Shutdown() = 0;
  virtual b8 Render() = 0;
};

#endif // !define ICE_RENDERING_RE_RENDERER_BACKEND_H_
