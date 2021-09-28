
#ifndef ICE_RENDERER_RENDERER_H_
#define ICE_RENDERER_RENDERER_H_
// TODO : Define API agnostic calls

#include "defines.h"
#include "renderer/renderer_backend.h"
#include "renderer/shader.h"
#include "renderer/mesh.h"
#include "renderer/vulkan/vulkan_material.h"
#include "renderer/material.h"
#include "core/ecs_components.h"

#include <glm/glm.hpp>
#include <vector>

class IceRenderer
{
public:
  enum IceRenderingAPI
  {
    Vulkan, // Ivk*
    //OpenGL, // Igl*
    //DirectX, // Idx*
    Invalid_API = -1
  };

//=================================================
// Variables
//=================================================
public:
  RendererBackend* backend = nullptr;
//private:
  IceRenderingAPI activeAPI = Invalid_API;
  std::vector<IceMaterial> materials;
  std::vector<IceShader> shaders;
  std::vector<mesh_t> meshes;

//=================================================
// Functions
//=================================================
public:
  void Initialize(IceRenderingAPI _api);
  void Shutdown();

  void RenderFrame(IceRenderPacket* _packet);

  // Returns the index of the shader program
  // Creates a new shader if none match the given description
  u32 GetMaterial(std::vector<const char*> _shaderNames,
                  std::vector<IceShaderStageFlags> _shaderStages,
                  std::vector<const char*> _texStrings,
                  IceFlag _renderSettings = 0);


  u32 CreateMesh(const char* _model);

  void Resize(u32 _width = 0, u32 _height = 0);

  IceRenderContext* GetContext() { return backend->GetContext(); }

};

extern IceRenderer Renderer;

#endif // !RENDERER_RENDERER_H
