
#include "asserts.h"
#include "defines.h"
#include "logger.h"

#include "renderer/vulkan/vulkan_shader.h"
#include "renderer/vulkan/vulkan_context.h"
#include "platform/file_system.h"

#include <string>
#include <vector>

IvkShader::IvkShader(IceRenderContext* _rContext,
                     const std::vector<const char*> _shaderNames,
                     const std::vector<IceShaderStageFlags> _shaderStages)
{
  info.sources = _shaderNames;
  std::vector<VkShaderModule> modules = GetShaders(_rContext, _shaderStages);

  // prepare & create graphics pipeline

  for (auto& m : modules)
  {
    vkDestroyShaderModule(_rContext->device, m, _rContext->allocator);
  }
}

IvkShader::~IvkShader()
{
  
}

std::vector<VkShaderModule> IvkShader::GetShaders(
    IceRenderContext* _rContext, const std::vector<IceShaderStageFlags> _shaderStages)
{
  u32 count = info.sources.size();
  std::vector<VkShaderModule> modules;
  for (u32 i = 0; i < count; i++)
  {
    LogDebug("Shader : %s", info.sources[i]);
    IceShaderStageFlags stages = _shaderStages[i];

    // TODO : Make more easily extendable?
    if (stages & Ice_Shader_Vert)
      modules.push_back(LoadShader(_rContext, info.sources[i], Ice_Shader_Vert));
    if (stages & Ice_Shader_Frag)
      modules.push_back(LoadShader(_rContext, info.sources[i], Ice_Shader_Frag));
    if (stages & Ice_Shader_Comp)
      modules.push_back(LoadShader(_rContext, info.sources[i], Ice_Shader_Comp));
  }

  return modules;
}

VkShaderModule IvkShader::LoadShader(
    IceRenderContext* _rContext, const char* _name, IceShaderStageFlags _stage)
{
  std::string shaderDir(ICE_RESOURCE_SHADER_DIR);
  shaderDir.append(_name);
  std::string layoutDir = shaderDir;

  switch (_stage)
  {
  case Ice_Shader_Vert:
    shaderDir.append(".vspv");
    layoutDir.append(".vlayout");
    break;
  case Ice_Shader_Frag:
    shaderDir.append(".fspv");
    layoutDir.append(".flayout");
    break;
  case Ice_Shader_Comp:
    shaderDir.append(".cspv");
    layoutDir.append(".clayout");
    break;
  default:
    LogError("Shader stage %u not recognized", _stage);
    return VK_NULL_HANDLE;
  }

  LogDebug("Load Shader : %s", shaderDir.c_str());

  std::vector<char> source = FileSystem::LoadFile(shaderDir.c_str());
  VkShaderModuleCreateInfo createInfo {};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = source.size();
  createInfo.pCode = reinterpret_cast<uint32_t*>(source.data());

  VkShaderModule module;
  IVK_ASSERT(vkCreateShaderModule(_rContext->device, &createInfo, _rContext->allocator, &module),
             "Failed to create shader module %s", shaderDir.c_str());

  return module;
}

