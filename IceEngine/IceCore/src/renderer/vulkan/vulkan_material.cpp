
#include "asserts.h"
#include "defines.h"
#include "logger.h"

#include "renderer/vulkan/vulkan_material.h"
#include "renderer/vulkan/vulkan_context.h"
#include "platform/file_system.h"

#include <string>
#include <vector>

IvkMaterial::IvkMaterial(IceRenderContext* _rContext,
                     const std::vector<const char*> _shaderNames,
                     const std::vector<IceShaderStageFlags> _shaderStages)
{
  LogDebug("Creating material");
  info.sources = _shaderNames;
  std::vector<IvkShader> shaders = GetShaders(_rContext, _shaderStages);

  CreateDescriptorSetLayout(_rContext, shaders);
  CreateDescriptorSet(_rContext);
  UpdateDescriptorSet(_rContext);

  for (auto& s : shaders)
  {
    vkDestroyShaderModule(_rContext->device, s.module, _rContext->allocator);
  }
  shaders.clear();
  LogDebug("Material created");

  Shutdown(_rContext);
}

void IvkMaterial::Shutdown(IceRenderContext* _rContext)
{
  vkDestroyDescriptorSetLayout(_rContext->device, descriptorSetLayout, _rContext->allocator);
}

std::vector<IvkShader> IvkMaterial::GetShaders(
    IceRenderContext* _rContext, const std::vector<IceShaderStageFlags> _shaderStages)
{
  u32 count = info.sources.size();
  std::vector<IvkShader> shaders;
  for (u32 i = 0; i < count; i++)
  {
    LogInfo("Shader : %s", info.sources[i]);
    IceShaderStageFlags stages = _shaderStages[i];

    // TODO : Make more easily extendable?
    if (stages & Ice_Shader_Vert)
      shaders.push_back(LoadShader(_rContext, info.sources[i], Ice_Shader_Vert));
    if (stages & Ice_Shader_Frag)
      shaders.push_back(LoadShader(_rContext, info.sources[i], Ice_Shader_Frag));
    if (stages & Ice_Shader_Comp)
      shaders.push_back(LoadShader(_rContext, info.sources[i], Ice_Shader_Comp));
  }

  return shaders;
}

IvkShader IvkMaterial::LoadShader(
    IceRenderContext* _rContext, const char* _name, IceShaderStageFlags _stage)
{
  IvkShader shader;

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
    return {0, VK_NULL_HANDLE, {}};
  }

  LogInfo("Load Shader : %s", shaderDir.c_str());

  CreateShaderModule(_rContext, shader, shaderDir.c_str());
  FillShaderBindings(shader, layoutDir.c_str());
  shader.stage = _stage;

  return shader;
}

void IvkMaterial::CreateShaderModule(
    IceRenderContext* _rContext, IvkShader& _shader, const char* _directory)
{
  std::vector<char> source = FileSystem::LoadFile(_directory);
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = source.size();
  createInfo.pCode = reinterpret_cast<uint32_t*>(source.data());

  IVK_ASSERT(vkCreateShaderModule(_rContext->device,
    &createInfo,
    _rContext->allocator,
    &_shader.module),
    "Failed to create shader module %s", _directory);
}

void IvkMaterial::FillShaderBindings(IvkShader& _shader, const char* _directory)
{
  // TODO : Implement a proper parser to extract shader bindings
  std::vector<char> layoutSource = FileSystem::LoadFile(_directory);
  std::string layout(layoutSource.data());
  u32 i = 0;
  while (layout[i] == 'b' || layout[i] == 's')
  {
    switch (layout[i])
    {
    case 'b':
      _shader.bindings.push_back(Ice_Shader_Binding_Buffer);
      break;
    case 's':
      _shader.bindings.push_back(Ice_Shader_Binding_Image);
      break;
    }

    i++;
  }
}

VkShaderStageFlagBits ivkstage(IceShaderStageFlags _stage)
{
  switch (_stage)
  {
  case Ice_Shader_Vert:
    return VK_SHADER_STAGE_VERTEX_BIT;
  case Ice_Shader_Frag:
    return VK_SHADER_STAGE_FRAGMENT_BIT;
  case Ice_Shader_Comp:
    return VK_SHADER_STAGE_COMPUTE_BIT;
  default:
    return VK_SHADER_STAGE_VERTEX_BIT;
  }
}

void IvkMaterial::CreateDescriptorSetLayout(
    IceRenderContext* _rContext, const std::vector<IvkShader>& _shaders)
{
  u32 bindingIndex = 0;
  std::vector<VkDescriptorSetLayoutBinding> stageBindings;
  VkDescriptorSetLayoutBinding binding{};
  binding.descriptorCount = 1;
  binding.pImmutableSamplers = nullptr;

  // Add shader bindings from all the program's shaders
  for (const auto& s : _shaders)
  {
    binding.stageFlags = ivkstage(s.stage);
    for (u32 i = 0; i < s.bindings.size(); i++)
    {
      switch (s.bindings[i])
      {
      case Ice_Shader_Binding_Buffer:
        binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        break;
      case Ice_Shader_Binding_Image:
        binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        break;
      }

      binding.binding = bindingIndex++;
      stageBindings.push_back(binding);
      info.bindings.push_back(s.bindings[i]);
    }
  }

  VkDescriptorSetLayoutCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  createInfo.bindingCount = static_cast<uint32_t>(stageBindings.size());
  createInfo.pBindings = stageBindings.data();

  IVK_ASSERT(vkCreateDescriptorSetLayout(_rContext->device,
                                         &createInfo,
                                         _rContext->allocator,
                                         &descriptorSetLayout),
             "Failed to create descriptor set layout");
}

void IvkMaterial::CreateDescriptorSet(IceRenderContext* _rContext)
{
  VkDescriptorSetAllocateInfo allocInfo {};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = _rContext->descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &descriptorSetLayout;

  IVK_ASSERT(vkAllocateDescriptorSets(_rContext->device, &allocInfo, &descriptorSet),
             "Failed to allocate descriptor set");
}

void IvkMaterial::UpdateDescriptorSet(IceRenderContext* _rContext)
{
  //std::vector<VkWriteDescriptorSet> writeSets(info.bindings.size());
  //std::vector<VkDescriptorImageInfo> imageInfos(info.bindings.size() - 1);

  //// Create the Uniform buffer
  //VkDescriptorBufferInfo mvpBufferInfo{};
  //mvpBufferInfo.buffer = mvpBuffer->GetBuffer();
  //mvpBufferInfo.offset = 0;
  //mvpBufferInfo.range = VK_WHOLE_SIZE;

  //// MVP matrix
  //writeSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  //writeSets[0].dstSet = shaderProgram.descriptorSet;
  //writeSets[0].dstBinding = 0;
  //writeSets[0].dstArrayElement = 0;
  //writeSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  //writeSets[0].descriptorCount = 1;
  //writeSets[0].pBufferInfo = &mvpBufferInfo;
  //writeSets[0].pImageInfo = nullptr;
  //writeSets[0].pTexelBufferView = nullptr;

  //// Fill info for each texture
  //for (u32 i = 1, imageBufferIdx = 0; i < info.bindings.size(); i++)
  //{
  //  if (info.bindings[i] == Ice_Shader_Binding_Image)
  //  {
  //    //u32 texIndex = GetTexture(shaderProgram.textureDirs[imageBufferIdx]);
  //    //u32 textureImageIdx = iceTextures[texIndex]->imageIndex;
  //    //VkDescriptorImageInfo iInfo{};
  //    //imageInfos[imageBufferIdx].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  //    //imageInfos[imageBufferIdx].sampler = iceImages[textureImageIdx]->sampler;
  //    //imageInfos[imageBufferIdx].imageView = iceImages[textureImageIdx]->view;

  //    //// Texture
  //    //writeSets[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  //    //writeSets[i].dstSet = shaderProgram.descriptorSet;
  //    //writeSets[i].dstBinding = i;
  //    //writeSets[i].dstArrayElement = 0;
  //    //writeSets[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  //    //writeSets[i].descriptorCount = 1;
  //    //writeSets[i].pBufferInfo = nullptr;
  //    //writeSets[i].pImageInfo = &imageInfos[texIndex];
  //    //writeSets[i].pTexelBufferView = nullptr;

  //    imageBufferIdx++;
  //  }
  //}

  //vkUpdateDescriptorSets(rContext->device, (u32)writeSets.size(), writeSets.data(), 0, nullptr);
}

