
#ifndef ICE_RENDERING_RENDERER_SETTINGS_H_
#define ICE_RENDERING_RENDERER_SETTINGS_H_

//#include <string>

enum IceRenderingApi
{
  Ice_Renderer_Vulkan,
  Ice_Renderer_OpenGL,
  Ice_Renderer_DirectX
};

struct IceRendererSettings
{
  IceRenderingApi api = Ice_Renderer_Vulkan;
  const char* lightingShader = "_light_blank"; // used by deferred rendering
};

#endif // !define ICE_RENDERING_RENDERER_SETTINGS_H_
