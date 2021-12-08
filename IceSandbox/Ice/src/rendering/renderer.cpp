
#include "defines.h"
#include "logger.h"

#include "rendering/renderer.h"
#include "rendering/vulkan/renderer_vulkan.h"

reIceRenderer reRenderer;

// Used temporarily until a direct call to ivkRenderer's Render() function can be established
reIvkRenderer* vkBackend;

b8 reIceRenderer::Initialize(reIceRendererSettings* _settings)
{
  // Unused until another rendering API is added
  // Need to find a way to *directly* call the backend's Render() (aka : without virtual functions)
  /*
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
  */
  vkBackend = new reIvkRenderer();

  if (!vkBackend->Initialize())
  {
    IceLogFatal("Failed to initialize renderer backend");
    return false;
  }

  return true;
}

b8 reIceRenderer::Shutdown()
{
  vkBackend->Shutdown();
  delete(vkBackend);
  return true;
}

b8 reIceRenderer::Render()
{
  ICE_ATTEMPT(vkBackend->Render());
  return true;
}
