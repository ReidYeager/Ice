
#include "defines.h"
#include "logger.h"

#include "rendering/renderer.h"

#include "rendering/vulkan/renderer_vulkan.h"

reIceRenderer reRenderer;

b8 reIceRenderer::Initialize(reIceRendererSettings* _settings)
{
  if (!backend.Initialize())
  {
    IceLogFatal("Failed to initialize renderer backend");
    return false;
  }

  return true;
}

b8 reIceRenderer::Shutdown()
{
  backend.Shutdown();
  return true;
}

b8 reIceRenderer::Render(IceCamera* _camera)
{
  ICE_ATTEMPT(backend.Render(_camera));
  return true;
}

u32 reIceRenderer::CreateMaterial(const std::vector<IceShaderInfo>& _shaders)
{
  return backend.CreateMaterial(_shaders);
}
