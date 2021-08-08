
#ifndef ICE_RENDERER_VULKAN_VULKAN_SHADER_H_
#define ICE_RENDERER_VULKAN_VULKAN_SHADER_H_

#include "defines.h"
#include "renderer/shader.h"

#include <vulkan/vulkan.h>
#include <vector>

class IvkShader
{
private:
  IceShaderInfo info;

  // TODO : Add pipeline settings
  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorSet descriptorSet;
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;

public:
  IvkShader(const std::vector<const char*> _shaders,
            const std::vector<IceShaderStageFlags> _shaderStages);
  ~IvkShader();

};

#endif // !ICE_RENDERER_VULKAN_VULKAN_SHADER_H_
