
#ifndef ICE_RENDERING_RENDER_CONTEXT_H_
#define ICE_RENDERING_RENDER_CONTEXT_H_

#include "defines.h"

#define ALIGN_FOR_SHADER __declspec(align(16)) // Shaders are 16-byte aligned

namespace Ice {

  enum RenderingApi
  {
    Renderer_Vulkan,
    Renderer_OpenGL,
    Renderer_DirectX
  };

}
#endif // !define ICE_RENDERING_RENDERER_SETTINGS_H_
