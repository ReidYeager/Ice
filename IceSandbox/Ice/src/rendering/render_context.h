
#ifndef ICE_RENDERING_RENDER_CONTEXT_H_
#define ICE_RENDERING_RENDER_CONTEXT_H_

#include "defines.h"

#include "math/vector.h"

#include <string>
#include <vector>

// Shaders are 16-byte aligned
#define ALIGN_FOR_SHADER __declspec(align(16))

//=========================
// Images
//=========================

enum IceImageType
{
  Ice_Image_Color,
  Ice_Image_Normal,
  Ice_Image_Depth,
  Ice_Image_Map
};

struct IceImage
{
  IceImageType type = Ice_Image_Color;
  IceHandle backendImage = (u32)ICE_NULL_HANDLE;
};

struct IceTexture
{
  std::string directory;
  IceImage image {};
};

//=========================
// Shader
//=========================

// Each value represents 16 bytes of information
#define ibi(x) Ice_Shader_Buffer_##x
enum IceShaderBufferInputs
{
  ibi(Custom_0),
  ibi(Custom_1),
  ibi(Custom_2),
  ibi(Custom_3),

  ibi(Custom_4),
  ibi(Custom_5),
  ibi(Custom_6),
  ibi(Custom_7),

  ibi(Custom_8),
  ibi(Custom_9),
  ibi(Custom_10),
  ibi(Custom_11),

  ibi(Count)
};
#undef ibi

#define ibi(x) #x
static const char* IceShaderBufferInputNames[Ice_Shader_Buffer_Count] =
{
  ibi(Custom_0),
  ibi(Custom_1),
  ibi(Custom_2),
  ibi(Custom_3),

  ibi(Custom_4),
  ibi(Custom_5),
  ibi(Custom_6),
  ibi(Custom_7),

  ibi(Custom_8),
  ibi(Custom_9),
  ibi(Custom_10),
  ibi(Custom_11),
};
#undef ibi

enum IceShaderStage
{
  Ice_Shader_Invalid,
  Ice_Shader_Vertex,
  Ice_Shader_Fragment
};

#define idt(x) Ice_Descriptor_Type_##x
enum IceShaderDescriptorType
{
  idt(Buffer),
  idt(Sampler2D),
  idt(SubpassInput),

  idt(Count)
};
#undef idt

#define idt(x) #x
static const char* IceDescriptorTypeNames[Ice_Descriptor_Type_Count] =
{
  idt(Buffer),
  idt(Sampler2D),
  idt(SubpassInput),

};
#undef idt

struct IceShaderDescriptor
{
  IceShaderDescriptorType type = Ice_Descriptor_Type_Sampler2D;
  u8 bindingIndex = 255; // The binding value within set 1 (0-254, 255 invalid)
  u32 data = 0; // Buffer size, image type, etc.
  IceHandle backendHandle = ICE_NULL_HANDLE; // For backend buffer, image, etc.
};

struct IceShader
{
  std::string directory;
  IceShaderStage stage;
  IceHandle backendShader = ICE_NULL_HANDLE;
  std::vector<IceShaderDescriptor> descriptors;
  u32 bufferParameters;
};

//=========================
// Material
//=========================

struct IceMaterial
{
  std::vector<IceHandle> shaderIndices;
  std::vector<IceShaderDescriptor> descriptors;
  IceHandle backendMaterial = ICE_NULL_HANDLE;
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
};

#endif // !define ICE_RENDERING_RENDERER_SETTINGS_H_
