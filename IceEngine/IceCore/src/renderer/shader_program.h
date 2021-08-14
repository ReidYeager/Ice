
#ifndef ICE_RENDERER_SHADER_PROGRAM_H_
#define ICE_RENDERER_SHADER_PROGRAM_H_

#include "defines.h"
#include "renderer/shader.h"
#include "renderer/vulkan/vulkan_context.h"
#include <vector>

// TODO : Completely remove iceShaderProgram_t

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

#include <vulkan/vulkan.h>
struct IceShader
{
  const char* name;
  IceShaderStageFlags stage;
  VkShaderModule module;
  std::vector<IceShaderBindingFlags> bindings;

  IceShader(const char* _name, IceShaderStageFlags _stage) : name(_name), stage(_stage) {}
  IceShader() {}
};

struct iceShaderProgram_t
{
  const char* name;
  IceShaderStageFlags stages;
  IcePipelineSettingFlags pipelineSettings;

  std::vector<u8> shaderIndices;

  // Binding format:
  // 1 uniform : Used for all data
  // * image samplers : Arbitrary number of textures
  std::vector<IceShaderBindingFlags> bindings;
  std::vector<const char*> textureDirs;
  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorSet descriptorSet;
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;

  iceShaderProgram_t(const char* _name, IcePipelineSettingFlags _settings = Ice_Pipeline_Default);
  VkPipeline GetPipeline(IceRenderContext* rContext);
  void DestroyRenderComponents(IceRenderContext* rContext);
  void Shutdown(IceRenderContext* rContext);
};

void ShadersShutdown(IceRenderContext* rContext);

// TODO : Refactor and move to RenderBackend
iceShaderProgram_t* GetShaderProgram(u32 _index);
u32 GetShaderProgram(
    IceRenderContext* rContext, const char* _name, IceShaderStageFlags _stages, std::vector<const char*> _texStrings,
    IcePipelineSettingFlags _settings = Ice_Pipeline_Default);

void CreateShaderProgram(IceRenderContext* rContext, const char* _name, IceShaderStageFlags _stages,
                         std::vector<const char*> _texStrings, IcePipelineSettingFlags _settings);

VkPipeline CreatePipeline(IceRenderContext* rContext, IceShader* _shaders, u32 _shaderCount, IceShaderStageFlags _stages,
                          VkPipelineLayout _layout, IcePipelineSettingFlags _settings);

u32 GetShader(IceRenderContext* rContext, const char* _name, IceShaderStageFlags _stage);

//void LoadShader(u32 _index);
void LoadShader(IceRenderContext* rContext, IceShader& _shader);

void ExtractShaderBindings(const char* _directory, IceShader& _shader);

void CreateDescriptorSetLayout(IceRenderContext* rContext, iceShaderProgram_t& _program);

void CreatePipelineLayout(IceRenderContext* rContext, iceShaderProgram_t& _program);

size_t PadBufferForGpu(IceRenderContext* rContext, size_t _original);


#endif // !RENDERER_SHADER_PROGRAM_H
