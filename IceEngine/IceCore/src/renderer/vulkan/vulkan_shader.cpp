
#include "defines.h"
#include "renderer/vulkan/vulkan_shader.h"

IvkShader::IvkShader(
    const std::vector<const char*> _shaders, const std::vector<IceShaderStageFlags> _shaderStages)
{
  info.sources = _shaders;
  // load source files
  // create shader modules
  // prepare & create graphics pipeline
}

IvkShader::~IvkShader()
{
  
}

