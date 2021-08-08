
#ifndef ICE_RENDERER_VULKAN_VULKAN_SHADER_H_
#define ICE_RENDERER_VULKAN_VULKAN_SHADER_H_

#include "defines.h"
#include "renderer/shader.h"
#include <vulkan/vulkan.h>
#include <vector>

struct IvkShader
{
  IceShaderStageFlags stage;
  VkShaderModule module;
  std::vector<IceShaderBindingFlagBits> bindings;
};

#endif // !ICE_RENDERER_VULKAN_VULKAN_SHADER_H_
