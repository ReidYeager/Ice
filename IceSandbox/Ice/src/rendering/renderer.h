
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

extern class reIceRenderer
{
private:
  IvkRenderer backend;
  std::vector<IceShader> shaders;
  std::vector<IceMaterial> materials;

public:
  b8 Initialize(const IceRendererSettings& _settings);
  b8 Shutdown();
  b8 Render(IceCamera* _camera);

  u32 CreateMesh(const char* _meshDir);
  void AddObjectToScene(IceObject* _object);

  void FillBuffer(IvkBuffer* _buffer, void* _data, u64 _size);

  void ReloadMaterials();

  u32 CreateMaterial(const std::vector<IceShader>& _shaders);
  void AssignMaterialTextures(IceHandle _material, std::vector<std::string> _images);
  u32 GetShader(const std::string& _directory, IceShaderStage _stage);
  void GetShaderDescriptors(IceShader& _shader);

} reRenderer;

#endif // !define ICE_RENDERING_RE_RENDERER_H_
