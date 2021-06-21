
#include "logger.h"
#include "renderer/shader_program.h"
#include "renderer/backend_context.h"
//#include "renderer/renderer_backend.h"
#include "platform/file_system.h"

#include <string>

iceShaderProgram_t::iceShaderProgram_t(const char* _name, IcePipelineSettingFlags _settings)
{
  name = _name;
  pipelineSettings = _settings;
  pipeline = VK_NULL_HANDLE;

  //CreateDescriptorSetLayout(this);
}

VkPipeline iceShaderProgram_t::GetPipeline()
{
  if (pipeline == VK_NULL_HANDLE)
  {
    std::vector<iceShader_t> shaders(shaderIndices.size());
    for (u32 i = 0; i < shaderIndices.size(); i++)
    {
      shaders[i] = rContext.shaders[shaderIndices[i]];
    }
    pipeline = CreatePipeline(shaders.data(), static_cast<u32>(shaders.size()), stages,
                              pipelineLayout, pipelineSettings);
  }

  return pipeline;
}

void iceShaderProgram_t::Shutdown()
{
  vkDestroyPipeline(rContext.device, pipeline, rContext.allocator);
  vkDestroyPipelineLayout(rContext.device, pipelineLayout, rContext.allocator);
  vkDestroyDescriptorSetLayout(rContext.device, descriptorSetLayout, rContext.allocator);
}

u32 GetShaderProgram(const char* _name, IceShaderStageFlags _stages,
                     IcePipelineSettingFlags _settings /*= Ice_Pipeline_Default*/)
{
  for (u32 i = 0; i < rContext.shaderPrograms.size(); i++)
  {
    iceShaderProgram_t& program = rContext.shaderPrograms[i];
    if (program.pipelineSettings == _settings && std::strcmp(_name, program.name) == 0)
    {
      return i;
    }
  }

  u32 index = static_cast<u32>(rContext.shaderPrograms.size());
  CreateShaderProgram(_name, _stages, _settings);
  return index;
}

void CreateShaderProgram(const char* _name, IceShaderStageFlags _stages,
                         IcePipelineSettingFlags _settings /*= Ice_Pipeline_Default*/)
{
  std::vector<u8> indices = {};
  u32 idx = 0;

  if (_stages & Ice_Shader_Stage_Vert)
  {
    idx = GetShader(_name, Ice_Shader_Stage_Vert);
    indices.push_back(idx);
  }

  if (_stages & Ice_Shader_Stage_Frag)
  {
    idx = GetShader(_name, Ice_Shader_Stage_Frag);
    indices.push_back(idx);
  }

  if (_stages & Ice_Shader_Stage_Comp)
  {
    idx = GetShader(_name, Ice_Shader_Stage_Comp);
    indices.push_back(idx);
  }

  iceShaderProgram_t program(_name, _settings);
  program.shaderIndices = indices;

  CreateDescriptorSetLayout(program);
  rContext.shaderPrograms.push_back(program);
}

VkShaderStageFlagBits IceToVkShaderStage(IceShaderStageFlags _stage)
{
  switch (_stage)
  {
  case Ice_Shader_Stage_Vert:
    return VK_SHADER_STAGE_VERTEX_BIT;
    break;
  case Ice_Shader_Stage_Frag:
    return VK_SHADER_STAGE_FRAGMENT_BIT;
    break;
  case Ice_Shader_Stage_Comp:
    return VK_SHADER_STAGE_COMPUTE_BIT;
    break;
  }

  return VK_SHADER_STAGE_VERTEX_BIT;
}

VkPipeline CreatePipeline(iceShader_t* _shaders, u32 _shaderCount, IceShaderStageFlags _stages,
                          VkPipelineLayout _layout, IcePipelineSettingFlags _settings)
{
  // Viewport State
  //=================================================
  VkViewport viewport;
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = (float)rContext.renderExtent.width;
  viewport.height = (float)rContext.renderExtent.height;
  viewport.minDepth = 0;
  viewport.maxDepth = 1;

  VkRect2D scissor{};
  scissor.extent = rContext.renderExtent;
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
  switch (_settings & Ice_Pipeline_Cull_Mode_Bits)
  {
  case Ice_Pipeline_Cull_Mode_Back:
    rasterStateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    break;
  case Ice_Pipeline_Cull_Mode_Front:
    rasterStateInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
    break;
  case Ice_Pipeline_Cull_Mode_Both:
    rasterStateInfo.cullMode = VK_CULL_MODE_FRONT_AND_BACK;
    break;
  case Ice_Pipeline_Cull_Mode_None:
    rasterStateInfo.cullMode = VK_CULL_MODE_NONE;
    break;
  }
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
  blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
    VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
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
  std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfos(_shaderCount);

  for (u32 i = 0; i < _shaderCount; i++)
  {
    shaderStageInfos[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfos[i].stage = IceToVkShaderStage(_shaders[i].stage);
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

  createInfo.layout = _layout;
  createInfo.renderPass = rContext.renderPass;

  VkPipeline tmpPipeline;
  ICE_ASSERT(vkCreateGraphicsPipelines(rContext.device, nullptr, 1, &createInfo,
                                       rContext.allocator, &tmpPipeline),
             "Failed to create graphics pipeline");

  for (u32 i = 0; i < _shaderCount; i++)
  {
    vkDestroyShaderModule(rContext.device, _shaders[i].module, rContext.allocator);
  }

  return tmpPipeline;
}

u32 GetShader(const char* _name, IceShaderStageFlags _stage)
{
  u32 i = 0;
  for (auto& shader : rContext.shaders)
  {
    if (std::strcmp(_name, shader.name) == 0 && shader.stage == _stage)
    {
      if (shader.module == VK_NULL_HANDLE)
      {
        LoadShader(shader);
      }
      return i;
    }
    i++;
  }

  i = static_cast<u32>(rContext.shaders.size());
  iceShader_t newShader(_name, _stage);
  rContext.shaders.push_back(newShader);
  LoadShader(rContext.shaders[i]);
  return i;
}

void LoadShader(iceShader_t& _shader)
{
  // TODO : Move to a constant value library
  std::string shaderDir = "res/Shaders/compiled/";
  shaderDir.append(_shader.name);
  std::string layoutDir(shaderDir);

  switch (_shader.stage)
  {
  case Ice_Shader_Stage_Vert:
    shaderDir.append(".vspv");
    layoutDir.append(".vlayout");
    break;
  case Ice_Shader_Stage_Frag:
    shaderDir.append(".fspv");
    layoutDir.append(".flayout");
    break;
  case Ice_Shader_Stage_Comp:
    shaderDir.append(".cspv");
    layoutDir.append(".clayout");
    break;
  }

  std::vector<char> sourceCode = FileSystem::LoadFile(shaderDir.c_str());
  VkShaderModuleCreateInfo createInfo {};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = sourceCode.size();
  createInfo.pCode = reinterpret_cast<const uint32_t*>(sourceCode.data());

  ICE_ASSERT(vkCreateShaderModule(rContext.device, &createInfo, nullptr, &_shader.module),
             "Failed to create shader module (%s)", shaderDir.c_str());

  ExtractShaderBindings(layoutDir.c_str(), _shader);
}

void ExtractShaderBindings(const char* _directory, iceShader_t& _shader)
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
      _shader.bindings.push_back(Ice_Shader_Binding_Combined_Sampler);
      break;
    }

    i++;
  }
}

void CreateDescriptorSetLayout(iceShaderProgram_t& _program)
{
  u32 bindingIndex = 0;
  std::vector<VkDescriptorSetLayoutBinding> stageBindings;
  VkDescriptorSetLayoutBinding binding {};
  binding.descriptorCount = 1;
  binding.pImmutableSamplers = nullptr;

  // Add shader bindings from all the program's shaders
  for (const auto& s : _program.shaderIndices)
  {
    iceShader_t& shader = rContext.shaders[s];
    binding.stageFlags = IceToVkShaderStage(shader.stage);
    for (u32 i = 0; i < shader.bindings.size(); i++)
    {
      switch (shader.bindings[i])
      {
      case Ice_Shader_Binding_Buffer:
        binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        break;
      case Ice_Shader_Binding_Combined_Sampler:
        binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        break;
      }

      binding.binding = bindingIndex++;
      stageBindings.push_back(binding);
      _program.bindings.push_back(shader.bindings[i]);
    }
  }

  VkDescriptorSetLayoutCreateInfo createInfo {};
  createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  createInfo.bindingCount = static_cast<uint32_t>(stageBindings.size());
  createInfo.pBindings = stageBindings.data();

  ICE_ASSERT(vkCreateDescriptorSetLayout(rContext.device, &createInfo, rContext.allocator,
                                         &_program.descriptorSetLayout),
             "Failed to create descriptor set layout for %s", _program.name);

  VkPipelineLayoutCreateInfo layoutInfo {};
  layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layoutInfo.setLayoutCount = 1;
  layoutInfo.pSetLayouts = &_program.descriptorSetLayout;
  layoutInfo.pushConstantRangeCount = 0;
  layoutInfo.pPushConstantRanges = nullptr;

  ICE_ASSERT(vkCreatePipelineLayout(rContext.device, &layoutInfo, rContext.allocator,
                                    &_program.pipelineLayout),
             "Failed to create pipeline layout for %s", _program.name);
}

size_t PadBufferForGpu(size_t _original)
{
  size_t alignment = rContext.gpu.properties.limits.minUniformBufferOffsetAlignment;
  size_t alignedSize = _original;
  if (alignment > 0)
  {
    alignedSize = (alignedSize + alignment - 1) & ~(alignment - 1);
  }
  return alignedSize;
}
