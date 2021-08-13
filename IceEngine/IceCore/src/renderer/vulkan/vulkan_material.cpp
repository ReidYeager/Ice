
#include "asserts.h"
#include "defines.h"
#include "logger.h"

#include "renderer/vulkan/vulkan_material.h"
#include "renderer/vulkan/vulkan_context.h"
#include "platform/file_system.h"

#include <string>
#include <vector>

// NOTE : Material's payload needs to be filled before use
IvkMaterial::IvkMaterial(IceRenderContext* _rContext,
                         const std::vector<const char*> _shaderNames,
                         const std::vector<IceShaderStageFlags> _shaderStages)
{
  LogDebug("Creating material");
  info.sources = _shaderNames;
  std::vector<IvkShader> shaders = GetShaders(_rContext, _shaderStages);

  CreateDescriptorSetLayout(_rContext, shaders);
  CreateDescriptorSet(_rContext);

  CreatePipelineLayout(_rContext);
  CreatePipeline(_rContext, shaders);

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
  LogDebug("Destroying material");
  vkDestroyPipeline(_rContext->device, pipeline, _rContext->allocator);
  vkDestroyPipelineLayout(_rContext->device, pipelineLayout, _rContext->allocator);
  vkDestroyDescriptorSetLayout(_rContext->device, descriptorSetLayout, _rContext->allocator);
  buffer->FreeBuffer(_rContext);
  delete(buffer);
}

void IvkMaterial::UpdatePayload(IceRenderContext* _rContext,
                                std::vector<const char*> _images,
                                IvkBuffer* _buffer /*= nullptr*/)
{
  std::vector<VkWriteDescriptorSet> writeSets(_images.size() + 1);
  std::vector<VkDescriptorImageInfo> imageInfos(_images.size());
  u32 writeIndex = 0;
  u32 imageIndex = 0;

  if (_buffer != nullptr)
    buffer = _buffer;
  if (buffer == nullptr)
    buffer = new IvkBuffer(_rContext,
                           1,
                           VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  // Bind the buffer
  VkDescriptorBufferInfo mvpBufferInfo{};
  mvpBufferInfo.buffer = buffer->GetBuffer();
  mvpBufferInfo.offset = 0;
  mvpBufferInfo.range = VK_WHOLE_SIZE;

  writeSets[writeIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writeSets[writeIndex].dstSet = descriptorSet;
  writeSets[writeIndex].dstBinding = writeIndex;
  writeSets[writeIndex].dstArrayElement = 0;
  writeSets[writeIndex].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  writeSets[writeIndex].descriptorCount = 1;
  writeSets[writeIndex].pBufferInfo = &mvpBufferInfo;
  writeSets[writeIndex].pImageInfo = nullptr;
  writeSets[writeIndex].pTexelBufferView = nullptr;
  writeIndex++;

  // Bind the images
  for (const auto& image : _images)
  {
    //imageInfos[imageIndex].imageLayout = image.layout;
    //imageInfos[imageIndex].imageView = image.view;
    //imageInfos[imageIndex].sampler = image.sampler;

    writeSets[writeIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeSets[writeIndex].dstSet = descriptorSet;
    writeSets[writeIndex].dstBinding = writeIndex;
    writeSets[writeIndex].dstArrayElement = 0;
    writeSets[writeIndex].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeSets[writeIndex].descriptorCount = 1;
    writeSets[writeIndex].pBufferInfo = nullptr;
    writeSets[writeIndex].pImageInfo = &imageInfos[imageIndex];
    writeSets[writeIndex].pTexelBufferView = nullptr;

    imageIndex++;
    writeIndex++;
  }

  vkUpdateDescriptorSets(_rContext->device, (u32)writeSets.size(), writeSets.data(), 0, nullptr);
}

void IvkMaterial::Render(VkCommandBuffer& _command)
{
  vkCmdBindPipeline(_command, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  vkCmdBindDescriptorSets(
      _command, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
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

void IvkMaterial::CreatePipelineLayout(IceRenderContext* _rContext)
{
  VkPipelineLayoutCreateInfo createInfo {};
  createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  createInfo.setLayoutCount = 1;
  createInfo.pSetLayouts = &descriptorSetLayout;
  createInfo.pushConstantRangeCount = 0;
  createInfo.pPushConstantRanges = nullptr;

  IVK_ASSERT(vkCreatePipelineLayout(
                 _rContext->device, &createInfo, _rContext->allocator, &pipelineLayout),
             "Failed to create pipeline layout");
}

void IvkMaterial::CreatePipeline(IceRenderContext* _rContext, std::vector<IvkShader> _shaders)
{
  // Viewport State
  //=================================================
  VkViewport viewport;
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = (float)_rContext->renderExtent.width;
  viewport.height = (float)_rContext->renderExtent.height;
  viewport.minDepth = 0;
  viewport.maxDepth = 1;

  VkRect2D scissor{};
  scissor.extent = _rContext->renderExtent;
  scissor.offset = { 0, 0 };

  VkPipelineViewportStateCreateInfo viewportStateInfo{};
  viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportStateInfo.viewportCount = 1;
  viewportStateInfo.pViewports = &viewport;
  viewportStateInfo.scissorCount = 1;
  viewportStateInfo.pScissors = &scissor;

  // Vertex Input State
  //=================================================
  const auto vertexInputBindingDesc = vertex_t::GetBindingDescription();
  const auto vertexInputAttribDesc = vertex_t::GetAttributeDescriptions();

  VkPipelineVertexInputStateCreateInfo vertexInputStateInfo{};
  vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  //vertexInputStateInfo.vertexAttributeDescriptionCount = 0;
  vertexInputStateInfo.vertexAttributeDescriptionCount = static_cast<u32>(vertexInputAttribDesc.size());
  vertexInputStateInfo.pVertexAttributeDescriptions = vertexInputAttribDesc.data();
  //vertexInputStateInfo.vertexBindingDescriptionCount = 0;
  vertexInputStateInfo.vertexBindingDescriptionCount = 1;
  vertexInputStateInfo.pVertexBindingDescriptions = &vertexInputBindingDesc;

  // Input Assembly State
  //=================================================
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo{};
  inputAssemblyStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssemblyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssemblyStateInfo.primitiveRestartEnable = VK_FALSE;

  // Rasterization State
  //=================================================
  VkPipelineRasterizationStateCreateInfo rasterStateInfo{};
  rasterStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
  rasterStateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterStateInfo.cullMode = VK_CULL_MODE_BACK_BIT;

  rasterStateInfo.rasterizerDiscardEnable = VK_TRUE;
  rasterStateInfo.lineWidth = 1.0f;
  rasterStateInfo.depthBiasEnable = VK_FALSE;
  rasterStateInfo.depthClampEnable = VK_FALSE;
  rasterStateInfo.rasterizerDiscardEnable = VK_FALSE;

  // Multisampling State
  //=================================================
  VkPipelineMultisampleStateCreateInfo multisampleStateInfo{};
  multisampleStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampleStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampleStateInfo.sampleShadingEnable = VK_FALSE;

  // Depth Stencil State
  //=================================================
  VkPipelineDepthStencilStateCreateInfo depthStateInfo{};
  depthStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStateInfo.depthTestEnable = VK_TRUE;
  depthStateInfo.depthWriteEnable = VK_TRUE;
  depthStateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
  depthStateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
  depthStateInfo.depthBoundsTestEnable = VK_FALSE;

  // Color Blend State
  //=================================================
  VkPipelineColorBlendAttachmentState blendAttachmentState{};
  blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                        VK_COLOR_COMPONENT_G_BIT |
                                        VK_COLOR_COMPONENT_B_BIT |
                                        VK_COLOR_COMPONENT_A_BIT;
  blendAttachmentState.blendEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo blendStateInfo{};
  blendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  blendStateInfo.logicOpEnable = VK_FALSE;
  blendStateInfo.attachmentCount = 1;
  blendStateInfo.pAttachments = &blendAttachmentState;

  // Dynamic States
  //=================================================
  VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
  dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicStateInfo.dynamicStateCount = 0;
  dynamicStateInfo.pDynamicStates = nullptr;

  // Shader Stages State
  //=================================================
  std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfos(_shaders.size());

  for (u32 i = 0; i < _shaders.size(); i++)
  {
    shaderStageInfos[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfos[i].stage = ivkstage(_shaders[i].stage);
    shaderStageInfos[i].module = _shaders[i].module;
    shaderStageInfos[i].pName = "main";
  }

  // Creation
  //=================================================
  VkGraphicsPipelineCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  createInfo.pViewportState = &viewportStateInfo;
  createInfo.pVertexInputState = &vertexInputStateInfo;
  createInfo.pInputAssemblyState = &inputAssemblyStateInfo;
  createInfo.pRasterizationState = &rasterStateInfo;
  createInfo.pMultisampleState = &multisampleStateInfo;
  createInfo.pDepthStencilState = &depthStateInfo;
  createInfo.pColorBlendState = &blendStateInfo;
  createInfo.pDynamicState = &dynamicStateInfo;

  createInfo.stageCount = static_cast<uint32_t>(shaderStageInfos.size());
  createInfo.pStages = shaderStageInfos.data();

  createInfo.layout = pipelineLayout;
  createInfo.renderPass = _rContext->renderPass;

  IVK_ASSERT(vkCreateGraphicsPipelines(
                 _rContext->device, nullptr, 1, &createInfo, _rContext->allocator, &pipeline),
             "Failed to create graphics pipeline");
}

