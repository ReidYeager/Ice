
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

typedef IceHandle IceBackendImage;

//=========================
// Shader
//=========================

// Each value represents 16 bytes of information
#define ibi(x) Ice_Shader_Buffer_##x
enum IceShaderBufferInputs
{
  ibi(Camera_View_Projection_Matrix_X),
  ibi(Camera_View_Projection_Matrix_Y),
  ibi(Camera_View_Projection_Matrix_Z),
  ibi(Camera_View_Projection_Matrix_W),

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
  ibi(Camera_View_Projection_Matrix_X),
  ibi(Camera_View_Projection_Matrix_Y),
  ibi(Camera_View_Projection_Matrix_Z),
  ibi(Camera_View_Projection_Matrix_W),

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

  idt(Count)
};
#undef idt

#define idt(x) #x
static const char* IceDescriptorTypeNames[Ice_Descriptor_Type_Count] =
{
  idt(Buffer),
  idt(Sampler2D)
};
#undef idt

struct IceShaderDescriptor
{
  u8 bindingIndex = 255; // The binding value within set 1 (0-254, 255 invalid)
  IceShaderDescriptorType type;
};

struct IceShaderBinding
{
  IceShaderDescriptor descriptor;
  void* backendData; // backend buffer, image, etc.
};

struct IceShader
{
  std::string directory;
  IceShaderStage stage;
  IceHandle backendShader;
  std::vector<IceShaderDescriptor> descriptors;
  std::vector<u32> bufferParameterIndices;
};

//=========================
// Material
//=========================

struct IceMaterial
{
  std::vector<IceHandle> shaderIndices;
  std::vector<IceShaderBinding> bindings;
  IceHandle backendMaterial;
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
