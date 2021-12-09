
#ifndef ICE_RENDERING_RE_RENDERER_H_
#define ICE_RENDERING_RE_RENDERER_H_

#include "defines.h"

#include "rendering/renderer_backend.h"

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

enum IceRenderingApi
{
  Ice_Renderer_Vulkan,
  Ice_Renderer_OpenGL,
  Ice_Renderer_DirectX
};

struct reIceRendererSettings
{
  IceRenderingApi api;
};

extern class reIceRenderer
{
private:
  reIceRendererBackend* backend;

public:
  b8 Initialize(reIceRendererSettings* _settings);
  b8 Shutdown();
  b8 Render(IceCamera* _camera);


} reRenderer;

#endif // !define ICE_RENDERING_RE_RENDERER_H_
