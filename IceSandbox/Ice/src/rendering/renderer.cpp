
#include "defines.h"
#include "logger.h"

#include "rendering/renderer.h"
#include "rendering/vulkan/vulkan_renderer.h"

#include <string>

reIceRenderer reRenderer;

b8 reIceRenderer::Initialize(const IceRendererSettings& _settings)
{
  if (!backend.Initialize(_settings))
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

void reIceRenderer::AssignMaterialTextures(IceHandle _material, std::vector<std::string> _images)
{
  // Retrieve images
  std::vector<IceHandle> texIndices(_images.size());

  for (u32 i = 0; i < _images.size(); i++)
  {
    texIndices[i] = backend.GetTexture(_images[i].c_str());
  }

  // Update all image samplers simultaneously
  backend.AssignMaterialTextures(_material, texIndices);
}

u32 reIceRenderer::CreateMesh(const char* _meshDir)
{
  return backend.CreateMesh(_meshDir);
}

void reIceRenderer::AddObjectToScene(IceObject* _object)
{
  backend.AddMeshToScene(_object);
}

void reIceRenderer::FillBuffer(IvkBuffer* _buffer, void* _data, u64 _size)
{
  backend.FillBuffer(_buffer, _data, _size);
}
