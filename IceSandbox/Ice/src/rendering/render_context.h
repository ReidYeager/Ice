
#ifndef ICE_RENDERING_RENDER_CONTEXT_H_
#define ICE_RENDERING_RENDER_CONTEXT_H_

#include "defines.h"

#include "math/vector.h"

#include <string>

// Shaders are 16-byte aligned
#define ALIGN_FOR_SHADER __declspec(align(16))

//=========================
// Images
//=========================

typedef IceHandle IceBackendImage;

//=========================
// Shader
//=========================

enum IceShaderStage
{
  Ice_Shader_Invalid,
  Ice_Shader_Vertex,
  Ice_Shader_Fragment ,
};

enum IceShaderDescriptorType
{
  Ice_Descriptor_Type_Buffer,
  Ice_Descriptor_Type_Sampled_Image
};

struct IceShaderDescriptor
{
  IceShaderDescriptorType type;
  u32 bufferByteSize;
  std::string imageName;
};

struct IceShaderInfo
{
  std::string directory;
  IceShaderStage stage;

  IceHandle backendModule;
};

//=========================
// Lights
//=========================

struct IceLightDirectional
{
  ALIGN_FOR_SHADER vec3 direction = { -1.0f, -1.0f, -1.0f };
  ALIGN_FOR_SHADER vec3 color = { 1.0f, 1.0f, 1.0f };
};

struct IceLightPoint
{
  ALIGN_FOR_SHADER vec3 position;
  ALIGN_FOR_SHADER vec3 color = { 1.0f, 1.0f, 1.0f };
};

//=========================
// Renderer
//=========================

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
