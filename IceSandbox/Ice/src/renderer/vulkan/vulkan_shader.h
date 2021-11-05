
#ifndef ICE_RENDERER_VULKAN_VULKAN_SHADER_H_
#define ICE_RENDERER_VULKAN_VULKAN_SHADER_H_

#include "defines.h"

#include "renderer/shader.h"
#include "renderer/shader_parameters.h"

#include <vector>
#include <vulkan/vulkan.h>

struct IvkShader
{
  IceShaderStageFlags stage;
  VkShaderModule module;
  std::vector<IceShaderBinding> bindings;
  u32 bufferIndex;
  IceShaderBufferParameterFlags bufferParameters;
  IceShaderImageParameterFlags imageParameters;
};

#endif // !ICE_RENDERER_VULKAN_VULKAN_SHADER_H_
