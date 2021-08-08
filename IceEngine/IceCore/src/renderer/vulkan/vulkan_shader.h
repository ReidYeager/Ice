
#ifndef ICE_RENDERER_VULKAN_VULKAN_SHADER_H_
#define ICE_RENDERER_VULKAN_VULKAN_SHADER_H_

#include "defines.h"
#include "renderer/vulkan/vulkan_context.h"
#include "renderer/shader.h"
#include <vulkan/vulkan.h>
#include <vector>

class IvkShader
{
private:
  IceShaderInfo info;

  // TODO : Add pipeline settings
  VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
  VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
  VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
  VkPipeline pipeline = VK_NULL_HANDLE;

public:
  IvkShader(IceRenderContext* _rContext,
            const std::vector<const char*> _shaderNames,
            const std::vector<IceShaderStageFlags> _shaderStages);
  ~IvkShader();

private:
  std::vector<VkShaderModule> GetShaders(
      IceRenderContext* _rContext, const std::vector<IceShaderStageFlags> _shaderStages);
  VkShaderModule LoadShader(
      IceRenderContext* _rContext, const char* _name, IceShaderStageFlags _stage);

};

#endif // !ICE_RENDERER_VULKAN_VULKAN_SHADER_H_
