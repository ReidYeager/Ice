
#ifndef ICE_RENDERING_RENDERER_CONTEXT_H_
#define ICE_RENDERING_RENDERER_CONTEXT_H_

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
    Shader_Unknown,
    Shader_Vertex,
    Shader_Fragment,
    Shader_Compute,
    // ...
  };

  struct Shader
  {
    ShaderTypes type;
    std::string fileDirectory;
    void* apiData[1];
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
    void* apiData[2];
  };

  //=========================
  // Renderer
  //=========================

  // The contents of this struct are currently in flux.
  // A permanent solution will be settled on eventually.
  struct FrameInformation
  {
    Ice::Material* materials;
    u32 materialCount;
  };

  enum RenderingApi
  {
    Renderer_Unknown,
    Renderer_Vulkan,
    Renderer_OpenGL,
    Renderer_DirectX
  };

  struct RendererSettings
  {
    //Ice::Renderer* existingRenderer = nullptr; // Used to setup resources for a new window
    Ice::RenderingApi api;
  };
}

#endif // !define ICE_RENDERING_RENDERER_CONTEXT_H_
