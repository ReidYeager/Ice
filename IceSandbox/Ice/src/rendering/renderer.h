
#ifndef ICE_RENDERING_RE_RENDERER_H_
#define ICE_RENDERING_RE_RENDERER_H_

#include "defines.h"

#include "rendering/render_context.h"
#include "rendering/vulkan/vulkan_renderer.h"
#include "core/camera.h"
#include "core/object.h"

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <vector>
#include <string>
#include <unordered_map>

extern class IceRenderer
{
private:
  IvkRenderer backend;
  std::vector<IceShader> shaders;
  std::vector<IceMaterial> materials;
  std::vector<IceTexture> textures;

public:
  b8 Initialize(const IceRendererSettings& _settings);
  b8 Shutdown();
  b8 Render(IceCamera* _camera);

  u32 CreateMesh(const char* _meshDir);
  void AddObjectToScene(IceObject* _object);

  void FillBuffer(IvkBuffer* _buffer, void* _data, u64 _size);

  b8 ReloadMaterials();

  IceHandle CreateMaterial(const std::vector<IceShader>& _shaders,
                           IceMaterialTypes _type,
                           u32 _subpassIndex);
  b8 PopulateMaterialDescriptors(IceMaterial* _material,
    std::unordered_map<IceShaderDescriptorType, std::vector<IceShaderDescriptor>>* _stacks = nullptr);
  void AssignMaterialTextures(IceHandle _material, std::vector<IceTexture> _textures);
  IceHandle GetShader(const std::string& _directory, IceShaderStage _stage);
  b8 GetShaderDescriptors(IceShader& _shader);

  b8 SetLightingMaterial(IceHandle _material);
  b8 SetMaterialBufferData(IceHandle _material, void* _data);

} renderer;

#endif // !define ICE_RENDERING_RE_RENDERER_H_
