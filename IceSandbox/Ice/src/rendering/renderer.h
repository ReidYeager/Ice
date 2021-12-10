
#ifndef ICE_RENDERING_RE_RENDERER_H_
#define ICE_RENDERING_RE_RENDERER_H_

#include "defines.h"

#include "rendering/vulkan/renderer_vulkan.h"
#include "core/camera.h"

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
  IvkRenderer backend;

public:
  b8 Initialize(reIceRendererSettings* _settings);
  b8 Shutdown();
  b8 Render(IceCamera* _camera);

  u32 CreateMaterial(const std::vector<IceShaderInfo>& _shaders);

  u32 CreateMesh(const char* _meshDir);
  void AddMeshToScene(u32 _meshIndex, u32 _materialIndex);

} reRenderer;

#endif // !define ICE_RENDERING_RE_RENDERER_H_
