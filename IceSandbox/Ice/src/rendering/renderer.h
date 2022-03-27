
#ifndef ICE_RENDERING_RENDERER_H_
#define ICE_RENDERING_RENDERER_H_

#include "defines.h"

#include <vector>
#include <string>

#define ALIGN_FOR_SHADER __declspec(align(16)) // Shaders are 16-byte aligned

namespace Ice {

  //=========================
  // Material
  //=========================

  enum ShaderTypes
  {
    Shader_Vertex,
    Shader_Fragment,
    Shader_Compute,
    // ...
  };

  struct Shader
  {
    const char* fileDirectory;
    ShaderTypes type;
  };

  struct MaterialSettings
  {
    std::vector<Ice::Shader> shaders;
    u32 subpassIndex = 0;
  };

  struct Material
  {
    // Used to hold arbitrary API information (pointer, int, etc.)
    // Vulkan uses struct pointer handles, OpenGL uses u32 handles
    void* apiData[4];

    std::vector<Ice::Shader> shaders;
  };

  //=========================
  // Renderer
  //=========================

  enum RenderingApi
  {
    Renderer_Vulkan,
    Renderer_OpenGL,
    Renderer_DirectX
  };

  struct RendererSettings
  {
    //Ice::Renderer* existingRenderer = nullptr; // Used to setup resources for a new window
    Ice::RenderingApi api;
  };

  class Renderer
  {
  public:
    virtual b8 Init(Ice::RendererSettings _settings) = 0;
    virtual b8 RenderFrame() = 0;
    virtual b8 Shutdown() = 0;

    virtual Ice::Material CreateMaterial(MaterialSettings _settings) = 0;
  };

}
#endif // !ICE_RENDERING_RENDERER_H_
