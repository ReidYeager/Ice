
#ifndef RENDERER_SHADER_PROGRAM_H
#define RENDERER_SHADER_PROGRAM_H 1

#include "defines.h"
#include <vector>

// NOTE : Move to defines?
enum IceShaderStageFlagBits
{
  Ice_Shader_Stage_Vert = 0x01, // vertex shader
  Ice_Shader_Stage_Frag = 0x02, // fragment shader
  Ice_Shader_Stage_Comp = 0x04  // compute shader
};
typedef IceFlag IceShaderStageFlags;

enum IceShaderBindingFlagBits
{
  Ice_Shader_Binding_Buffer = 0x00,
  Ice_Shader_Binding_Combined_Sampler = 0x01,
};
typedef IceFlag IceShaderBindingFlags;

enum IcePipelineSettingFlagBits
{
  Ice_Pipeline_Cull_Mode_None  = (0 << 0),
  Ice_Pipeline_Cull_Mode_Front = (1 << 0),
  Ice_Pipeline_Cull_Mode_Back  = (2 << 0),
  Ice_Pipeline_Cull_Mode_Both  = (3 << 0),
  Ice_Pipeline_Cull_Mode_Bits  = (3 << 0),

  // Default values
  Ice_Pipeline_Default = Ice_Pipeline_Cull_Mode_Back
};
typedef IceFlagExtended IcePipelineSettingFlags;

#ifdef ICE_VULKAN
#include <vulkan/vulkan.h>
struct iceShader_t
{
  const char* name;
  IceShaderStageFlags stage;
  VkShaderModule module;
  std::vector<IceShaderBindingFlags> bindings;
};
#else
struct iceShader_t
{
  const char* name;
  IceShaderStageFlags stage;
};
#endif // ICE_VULKAN

struct iceShaderProgram_t
{
  const char* name;
  IceShaderStageFlags stages;
  IceShaderBindingFlags pipelineSettingFlags;

  u8 vertIndex;
  u8 fragIndex;
  u8 compIndex;

  std::vector<IceShaderBindingFlags> bindings;
  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorSet descriptorSet;
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;

  iceShaderProgram_t(const char* _name, IcePipelineSettingFlags _settings = Ice_Pipeline_Default);
};

#endif // !RENDERER_SHADER_PROGRAM_H
