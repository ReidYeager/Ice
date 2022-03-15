
#ifndef ICE_RENDERING_RENDERER_H_
#define ICE_RENDERING_RENDERER_H_

#include "defines.h"

#include <vector>

#define ALIGN_FOR_SHADER __declspec(align(16)) // Shaders are 16-byte aligned

namespace Ice {

  // ==========
  // Renderer
  // ==========

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

  };

}
#endif // !ICE_RENDERING_RENDERER_H_
