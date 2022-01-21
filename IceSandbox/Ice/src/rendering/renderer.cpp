
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

u32 reIceRenderer::CreateMaterial(std::vector<IceShaderInfo>& _shaderInfos)
{
  std::vector<IceHandle> shaderIndices;

  // Load shaders =====
  for (auto& s : _shaderInfos)
  {
    b8 shaderFound = false;

    // Check for existing shader =====
    for (auto& existingShader : shaders)
    {
      if (existingShader.stage == s.stage &&
          existingShader.directory.compare(s.directory.c_str()) == 0)
      {
        shaderIndices.push_back(existingShader.backendModule);
        shaderFound = true;
        break;
      }
    }

    if (shaderFound)
      continue;

    // Create new shader =====
    s.backendModule = backend.CreateShader(s.directory, s.stage);
    shaderIndices.push_back(s.backendModule);

    shaders.push_back(s);
  }

  return backend.CreateMaterial(shaderIndices);
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

void reIceRenderer::ReloadMaterials()
{
  backend.ReloadMaterials();
}
