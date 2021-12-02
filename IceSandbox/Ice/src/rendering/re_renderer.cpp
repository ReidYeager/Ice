
#include "defines.h"
#include "logger.h"

#include "rendering/re_renderer.h"
#include "rendering/vulkan/re_renderer_vulkan.h"

reIceRenderer reRenderer;

b8 reIceRenderer::Initialize(reIceRendererSettings* _settings)
{
  switch (_settings->api)
  {
  case Ice_Renderer_Vulkan:
  {
    backend = new reIvkRenderer();
  } break;
  default:
  {
    IceLogFatal("The selected rendering API is not supported");
    return false;
  } break;
  }

  if (backend == nullptr)
  {
    IceLogFatal("Failed to create a renderer backend");
    return false;
  }

  if (!backend->Initialize())
  {
    IceLogFatal("Failed to initialize renderer backend");
    return false;
  }

  return true;
}

b8 reIceRenderer::Shutdown()
{
  backend->Shutdown();
  delete(backend);
  return true;
}

b8 reIceRenderer::Render()
{
  backend->Render();
  return true;
}
