
#ifndef ICE_RENDERER_SHADER_H_
#define ICE_RENDERER_SHADER_H_

#include "defines.h"
#include "renderer/buffer.h"
#include "renderer/shader_parameters.h"
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
