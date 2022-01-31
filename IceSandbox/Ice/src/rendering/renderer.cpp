
#include "defines.h"
#include "logger.h"

#include "rendering/renderer.h"
#include "rendering/vulkan/vulkan_renderer.h"

#include <string>

IceRenderer renderer;

b8 IceRenderer::Initialize(const IceRendererSettings& _settings)
{
  if (!backend.Initialize(_settings))
  {
    IceLogFatal("Failed to initialize renderer backend");
    return false;
  }

  return true;
}

b8 IceRenderer::Shutdown()
{
  backend.Shutdown();
  return true;
}

b8 IceRenderer::Render(IceCamera* _camera)
{
  ICE_ATTEMPT_BOOL(backend.Render(_camera));
  return true;
}

void IceRenderer::AssignMaterialTextures(IceHandle _material, std::vector<IceTexture> _textures)
{
  // Retrieve images
  std::vector<IceHandle> texIndices(_textures.size());

  for (u32 i = 0; i < _textures.size(); i++)
  {
    u32 index = ICE_NULL_HANDLE;

    // Get existing gexture =====
    for (const auto& t : textures)
    {
      if (_textures[i].directory.compare(t.directory) == 0)
      {
        index = t.image.backendImage;
        break;
      }
    }

    // Create new or use default =====
    if (index == ICE_NULL_HANDLE)
      index = backend.GetTexture(_textures[i].directory.c_str(), _textures[i].image.type);

    texIndices[i] = index;

  }

  //for (u32 desc = 0, tex = 0; desc < materials[_material].descriptors.size() && tex < _textures.size(); desc++)
  //{
  //  if (materials[_material].descriptors[desc].type != Ice_Descriptor_Type_Sampler2D)
  //    continue;

  //  IceHandle index = ICE_NULL_HANDLE;
  //  // Try to get existing texture =====
  //  for (const auto& t : textures)
  //  {
  //    if (_textures[tex].directory.compare(t.directory) == 0)
  //    {
  //      index = t.image.backendImage;
  //      break;
  //    }
  //  }

  //  // Create new or use default =====
  //  if (index == ICE_NULL_HANDLE)
  //    index = backend.GetTexture(_textures[tex].directory.c_str(), _textures[tex].image.type);

  //  texIndices[tex] = index;
  //}

  // Update all image samplers simultaneously
  backend.AssignMaterialTextures(materials[_material], texIndices);
  IceLogInfo("----");
}

b8 IceRenderer::SetLightingMaterial(IceHandle _material)
{
  return backend.SetDeferredLightingMaterial(_material);
}

u32 IceRenderer::CreateMesh(const char* _meshDir)
{
  return backend.CreateMesh(_meshDir);
}

void IceRenderer::AddObjectToScene(IceObject* _object)
{
  backend.AddMeshToScene(_object);
}

void IceRenderer::FillBuffer(IvkBuffer* _buffer, void* _data, u64 _size)
{
  backend.FillBuffer(_buffer, _data, _size);
}
