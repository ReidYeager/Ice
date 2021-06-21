
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

  iceShader_t(const char* _name, IceShaderStageFlags _stage) : name(_name), stage(_stage) {}
  iceShader_t() {}
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
  IcePipelineSettingFlags pipelineSettings;

  std::vector<u8> shaderIndices;

  std::vector<IceShaderBindingFlags> bindings;
  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorSet descriptorSet;
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;

  iceShaderProgram_t(const char* _name, IcePipelineSettingFlags _settings = Ice_Pipeline_Default);
  VkPipeline GetPipeline();
  void Shutdown();
};

// TODO : Refactor and move to RenderBackend
u32 GetShaderProgram(const char* _name, IceShaderStageFlags _stages,
                     IcePipelineSettingFlags _settings = Ice_Pipeline_Default);

void CreateShaderProgram(const char* _name, IceShaderStageFlags _stages,
                         IcePipelineSettingFlags _settings = Ice_Pipeline_Default);

VkPipeline CreatePipeline(iceShader_t* _shaders, u32 _shaderCount, IceShaderStageFlags _stages,
                          VkPipelineLayout _layout, IcePipelineSettingFlags _settings);

u32 GetShader(const char* _name, IceShaderStageFlags _stage);

//void LoadShader(u32 _index);
void LoadShader(iceShader_t& _shader);

void ExtractShaderBindings(const char* _directory, iceShader_t& _shader);

void CreateDescriptorSetLayout(iceShaderProgram_t& _program);

size_t PadBufferForGpu(size_t _original);


#endif // !RENDERER_SHADER_PROGRAM_H
