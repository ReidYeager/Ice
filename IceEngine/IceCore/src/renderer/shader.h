
#ifndef ICE_RENDERER_SHADER_H_
#define ICE_RENDERER_SHADER_H_

#include "defines.h"
#include "renderer/buffer.h"
#include <vector>

enum IceShaderStageFlagBits
{
  Ice_Shader_Vert = 0x01, // vertex shader
  Ice_Shader_Frag = 0x02, // fragment shader
  Ice_Shader_Comp = 0x04  // compute shader
};
typedef IceFlag IceShaderStageFlags;

enum IceShaderBinding
{
  Ice_Shader_Binding_Buffer,
  Ice_Shader_Binding_Image,

  Ice_Shader_Binding_Count,
  Ice_Shader_Binding_Invalid = -1
};

#define ISP(name, bit) Ice_Shader_Param_##name = (u64)1 << bit
// Represents information that the shaders may require as input
// Each enum represents 16 bytes
enum IceShaderBufferParameterFlagBits : IceFlagExtended
{
  // Make model matrix a guaranteed buffer input?
  // Would not require these definitions if it is guaranteed to be sent to the shader
  ISP(ModelMatrix_X, 0), // X vector of model matrix
  ISP(ModelMatrix_Y, 1), // Y vector of model matrix
  ISP(ModelMatrix_Z, 2), // Z vector of model matrix
  ISP(ModelMatrix_W, 3), // W vector of model matrix
  
  ISP(VpMatrix_X, 4), // X vector of combined View/Projection matrix
  ISP(VpMatrix_Y, 5), // Y vector of combined View/Projection matrix
  ISP(VpMatrix_Z, 6), // Z vector of combined View/Projection matrix
  ISP(VpMatrix_W, 7), // W vector of combined View/Projection matrix

  ISP(ViewMatrix_X, 8),  // X vector of view matrix
  ISP(ViewMatrix_Y, 9),  // Y vector of view matrix
  ISP(ViewMatrix_Z, 10), // Z vector of view matrix
  ISP(ViewMatrix_W, 11), // W vector of view matrix

  ISP(ProjectionMatrix_X, 12), // X vector of projection matrix
  ISP(ProjectionMatrix_Y, 13), // Y vector of projection matrix
  ISP(ProjectionMatrix_Z, 14), // Z vector of projection matrix
  ISP(ProjectionMatrix_W, 15), // W vector of projection matrix

  ISP(User0, 52),
  ISP(User1, 53),
  ISP(User2, 54),
  ISP(User3, 55),

  ISP(User4, 56),
  ISP(User5, 57),
  ISP(User6, 58),
  ISP(User7, 59),
  
  ISP(User8,  60),
  ISP(User9,  61),
  ISP(User10, 62),
  ISP(User11, 63),
};
typedef IceFlagExtended IceShaderBufferParameterFlags;
#undef ISP

struct IceShaderInfo
{
  std::vector<const char*> sourceNames;
  std::vector<IceShaderStageFlags> sourceStages;
  std::vector<u64> sourceLastModifiedTimes;
  IceShaderStageFlags stages;
  std::vector<IceShaderBinding> bindings;
  IceShaderBufferParameterFlags bufferParameterFlags;
  std::vector<iceImage_t*> textures;
};

enum IcePipelineSettingFlagBits
{
  Ice_Pipeline_Cull_Mode_None = (0 << 0),
  Ice_Pipeline_Cull_Mode_Front = (1 << 0),
  Ice_Pipeline_Cull_Mode_Back = (2 << 0),
  Ice_Pipeline_Cull_Mode_Both = (3 << 0),
  Ice_Pipeline_Cull_Mode_Bits = (3 << 0),

  // Default values
  Ice_Pipeline_Default = Ice_Pipeline_Cull_Mode_Back
};
typedef IceFlagExtended IcePipelineSettingFlags;

#include <vulkan/vulkan.h>
struct IceShader
{
  const char* name;
  IceShaderStageFlags stage;
  VkShaderModule module;
  std::vector<IceShaderBinding> bindings;

  IceShader(const char* _name, IceShaderStageFlags _stage) : name(_name), stage(_stage) {}
  IceShader() {}
};

#endif // !ICE_RENDERER_SHADER_H_
