
#include "asserts.h"
#include "defines.h"
#include "logger.h"

#include "renderer/vulkan/vulkan_material.h"
#include "renderer/vulkan/vulkan_context.h"
#include "renderer/image.h"
#include "renderer/buffer.h"
#include "platform/file_system.h"

#include <string>
#include <vector>
#include <sys/stat.h>

void IvkMaterial_T::Initialize(IceRenderContext* _rContext,
                               const std::vector<const char*> _shaderNames,
                               const std::vector<IceShaderStageFlags> _shaderStages,
                               IceBuffer _buffer /*= nullptr*/)
{
  LogDebug("Creating material");
  info.sourceNames = _shaderNames;
  info.sourceStages = _shaderStages;

  // Create components
  shaders = GetShaders(_rContext);

  CreateDescriptorSetLayout(_rContext, shaders);
  CreateDescriptorSet(_rContext);

  CreateFragileComponents(_rContext);

  // Create a buffer if none exists
  if (_buffer != nullptr)
  {
    materialBuffer = _buffer;
  }
  else
  {
    u64 size;
    u64 RequiredBuffers = info.bufferParameterFlags;
    // Count the number of set buffer parameter flags
    for (size = 0; RequiredBuffers; size++)
    {
      RequiredBuffers &= RequiredBuffers - 1;
    }

    materialBuffer = new IvkBuffer(_rContext,
                                   size * 16,
                                   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                                      VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  }

  // Forces an update of the the descriptor set
  UpdateImages(_rContext, {}, nullptr, 0);

  LogDebug("Material created");
}

void IvkMaterial_T::Shutdown(IceRenderContext* _rContext)
{
  LogDebug("Destroying material");

  DestroyFragileComponents(_rContext);
  vkDestroyDescriptorSetLayout(_rContext->device, descriptorSetLayout, _rContext->allocator);

  // Clear shader modules
  for (IvkShader s : shaders)
  {
    vkDestroyShaderModule(_rContext->device, s.module, _rContext->allocator);
  }
  shaders.clear();

  if (materialBuffer != nullptr)
  {
    materialBuffer->Free(_rContext);
    delete(materialBuffer);
  }
}

void IvkMaterial_T::DestroyFragileComponents(IceRenderContext* _rContext)
{
  vkDestroyPipeline(_rContext->device, pipeline, _rContext->allocator);
  vkDestroyPipelineLayout(_rContext->device, pipelineLayout, _rContext->allocator);
}

void IvkMaterial_T::CreateFragileComponents(IceRenderContext* _rContext)
{
  CreatePipelineLayout(_rContext);
  CreatePipeline(_rContext, shaders);
}

// Need to move to a dedicated shader module manager
void IvkMaterial_T::UpdateSources(IceRenderContext* _rContext)
{
  bool shouldUpdate = false;

  for (u32 i = 0; i < info.sourceNames.size();i++)
  {
    // Construct the file name
    std::string shaderDir(ICE_RESOURCE_SHADER_DIR);
    shaderDir.append(info.sourceNames[i]);

    switch (info.sourceStages[i])
    {
    case Ice_Shader_Vert:
      shaderDir.append(".vert.spv");
      break;
    case Ice_Shader_Frag:
      shaderDir.append(".frag.spv");
      break;
    case Ice_Shader_Comp:
      shaderDir.append(".comp.spv");
      break;
    default:
      LogError("Shader stage %u not recognized", info.sourceStages[i]);
    }

    // If the time modified is more recent than time loaded
    struct _stat result;
    if (_stat(shaderDir.c_str(), &result) == 0 &&
        result.st_mtime != info.sourceLastModifiedTimes[i])
    {
      LogInfo("Shader --- %20s --- loaded: %u, new: %u",
              shaderDir.c_str(),
              info.sourceLastModifiedTimes[i],
              result.st_mtime);

      info.sourceLastModifiedTimes[i] = result.st_mtime;
      shouldUpdate = true;
    }
  }

  // Reload shaders
  if (shouldUpdate)
  {
    vkDeviceWaitIdle(_rContext->device);
    DestroyFragileComponents(_rContext);

    for (IvkShader s : shaders)
    {
      vkDestroyShaderModule(_rContext->device, s.module, _rContext->allocator);
    }
    shaders.clear();
    shaders = GetShaders(_rContext);

    CreateFragileComponents(_rContext);
  }
}

void IvkMaterial_T::UpdateBuffer(IceRenderContext* _rContext, void* _userData, IceShaderBufferParameterFlags _userParameterFlags)
{
  // Update user data
  if (_userData != nullptr)
  {
    // Update selected user parameters
    FillShaderBuffer(_rContext, materialBuffer, _userData, _userParameterFlags, info.bufferParameterFlags);
  }
}

void IvkMaterial_T::UpdateImages(IceRenderContext* _rContext,
                                  std::vector<iceImage_t*> _images,
                                  void* _userData,
                                  IceShaderBufferParameterFlags _userParameterFlags)
{
  u64 imageCount = (_images.size() < info.textures.size()) ? _images.size() : info.textures.size();
  std::vector<VkWriteDescriptorSet> writeSets(imageCount + 1);
  std::vector<VkDescriptorImageInfo> imageInfos(imageCount);
  u32 writeIndex = 0;
  u32 imageIndex = 0;

  // Update user data
  UpdateBuffer(_rContext, _userData, _userParameterFlags);

  // Update non-user parameters

  // Bind the buffer
  VkDescriptorBufferInfo bufferInfo{};
  bufferInfo.buffer = materialBuffer->GetBuffer();
  bufferInfo.offset = 0;
  bufferInfo.range = VK_WHOLE_SIZE;

  writeSets[writeIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writeSets[writeIndex].dstSet = descriptorSet;
  writeSets[writeIndex].dstBinding = writeIndex;
  writeSets[writeIndex].dstArrayElement = 0;
  writeSets[writeIndex].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  writeSets[writeIndex].descriptorCount = 1;
  writeSets[writeIndex].pBufferInfo = &bufferInfo;
  writeSets[writeIndex].pImageInfo = nullptr;
  writeSets[writeIndex].pTexelBufferView = nullptr;
  writeIndex++;

  // Bind the images
  for (u32 imageIndex = 0; imageIndex < imageCount; imageIndex++, writeIndex++)
  {
    info.textures[imageIndex] = _images[imageIndex];
    imageInfos[imageIndex].imageLayout = info.textures[imageIndex]->layout;
    imageInfos[imageIndex].imageView = info.textures[imageIndex]->view;
    imageInfos[imageIndex].sampler = info.textures[imageIndex]->sampler;

    writeSets[writeIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeSets[writeIndex].dstSet = descriptorSet;
    writeSets[writeIndex].dstBinding = writeIndex;
    writeSets[writeIndex].dstArrayElement = 0;
    writeSets[writeIndex].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeSets[writeIndex].descriptorCount = 1;
    writeSets[writeIndex].pBufferInfo = nullptr;
    writeSets[writeIndex].pImageInfo = &imageInfos[imageIndex];
    writeSets[writeIndex].pTexelBufferView = nullptr;

    //writeIndex++;
  }

  vkUpdateDescriptorSets(_rContext->device, (u32)writeSets.size(), writeSets.data(), 0, nullptr);
}

void IvkMaterial_T::Render(VkCommandBuffer& _command,
                           const void* _modelMatrix,
                           const void* _viewProjMatrix)
{
  //LogInfo("Rendering %s, %s", info.sourceNames[0], info.sourceNames[1]);

  vkCmdBindPipeline(_command, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  vkCmdBindDescriptorSets(_command,
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          pipelineLayout,
                          0,
                          1,
                          &descriptorSet,
                          0,
                          nullptr);
  vkCmdPushConstants(_command, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, 64, _modelMatrix);
  vkCmdPushConstants(_command, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 64, 64, _viewProjMatrix);
}

std::vector<IvkShader> IvkMaterial_T::GetShaders(IceRenderContext* _rContext)
{
  // Load all input shaders
  u32 count = (u32)info.sourceNames.size();
  std::vector<IvkShader> shaders;
  for (u32 i = 0; i < count; i++)
  {
    LogInfo("Shader : %s", info.sourceNames[i]);
    IceShaderStageFlags stages = info.sourceStages[i];

    // Load each stage of the shader
    if (stages & Ice_Shader_Vert)
      shaders.push_back(LoadShader(_rContext, info.sourceNames[i], Ice_Shader_Vert));
    if (stages & Ice_Shader_Frag)
      shaders.push_back(LoadShader(_rContext, info.sourceNames[i], Ice_Shader_Frag));
    if (stages & Ice_Shader_Comp)
      shaders.push_back(LoadShader(_rContext, info.sourceNames[i], Ice_Shader_Comp));
  }

  return shaders;
}

IvkShader IvkMaterial_T::LoadShader(IceRenderContext* _rContext,
                                    const char* _name,
                                    IceShaderStageFlags _stage)
{
  IvkShader shader;

  // Construct the filename
  std::string shaderDir(ICE_RESOURCE_SHADER_DIR);
  shaderDir.append(_name);
  std::string layoutDir = shaderDir;

  switch (_stage)
  {
  case Ice_Shader_Vert:
    shaderDir.append(".vert.spv");
    layoutDir.append(".vlayout");
    break;
  case Ice_Shader_Frag:
    shaderDir.append(".frag.spv");
    layoutDir.append(".flayout");
    break;
  case Ice_Shader_Comp:
    shaderDir.append(".comp.spv");
    layoutDir.append(".clayout");
    break;
  default:
    LogError("Shader stage %u not recognized", _stage);
    return {0, VK_NULL_HANDLE, {}};
  }

  LogInfo("Load Shader : %s", shaderDir.c_str());

  // Load and process the shader and its bindings
  CreateShaderModule(_rContext, shader, shaderDir.c_str());
  FillShaderBindings(shader, layoutDir.c_str());
  shader.stage = _stage;

  // Set last modified time
  struct _stat result;
  if (_stat(shaderDir.c_str(), &result) == 0)
  {
    info.sourceLastModifiedTimes.push_back(result.st_mtime);
  }

  return shader;
}

void IvkMaterial_T::CreateShaderModule(IceRenderContext* _rContext,
                                       IvkShader& _shader,
                                       const char* _directory)
{
  // Retrieve the shader's source code
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

void SkipWhiteSpace(u64& _place, std::string& _layout)
{
  while (_place < _layout.length() && (_layout[_place] == ' ' ||
    _layout[_place] == '\t' ||
    _layout[_place] == '\n' ||
    _layout[_place] == '\r'))
  {
    _place++;
  }
}

IceShaderBufferParameterFlags StringToBufferParam(const char* _str)
{
  if (std::strcmp("ModelMatrix", _str) == 0)
    return Ice_Shader_Param_ModelMatrix;
  if (std::strcmp("VpMatrix", _str) == 0)
    return Ice_Shader_Param_VpMatrix;
  if (std::strcmp("ViewMatrix", _str) == 0)
    return Ice_Shader_Param_ViewMatrix;
  if (std::strcmp("ProjectionMatrix", _str) == 0)
    return Ice_Shader_Param_ProjectionMatrix;


  if (std::strcmp("User0", _str) == 0)
    return Ice_Shader_Param_User0;
  if (std::strcmp("User1", _str) == 0)
    return Ice_Shader_Param_User1;
  if (std::strcmp("User2", _str) == 0)
    return Ice_Shader_Param_User2;
  if (std::strcmp("User3", _str) == 0)
    return Ice_Shader_Param_User3;
  if (std::strcmp("User4", _str) == 0)
    return Ice_Shader_Param_User4;
  if (std::strcmp("User5", _str) == 0)
    return Ice_Shader_Param_User5;
  if (std::strcmp("User6", _str) == 0)
    return Ice_Shader_Param_User6;
  if (std::strcmp("User7", _str) == 0)
    return Ice_Shader_Param_User7;
  if (std::strcmp("User8", _str) == 0)
    return Ice_Shader_Param_User8;
  if (std::strcmp("User9", _str) == 0)
    return Ice_Shader_Param_User9;
  if (std::strcmp("User10", _str) == 0)
    return Ice_Shader_Param_User10;
  if (std::strcmp("User11", _str) == 0)
    return Ice_Shader_Param_User11;

  return 0;
}

IceShaderBufferParameterFlags GetNextBufferParameter(u64& _place, std::string& _layout)
{
  SkipWhiteSpace(_place, _layout);

  u64 start = _place;

  while (_place < _layout.length() &&
         _layout[_place] != '\n' &&
         _layout[_place] != '\r' &&
         _layout[_place] != ' '  &&
         _layout[_place] != ','  &&
         _layout[_place] != '}')
  {
    _place++;
  }

  return StringToBufferParam(_layout.substr(start, _place - start).c_str());
}

IceShaderBufferParameterFlags GetBufferParameters(u64& _place, std::string& _layout)
{
  while (_place < _layout.length() && _layout[_place] != '{')
  {
    _place++;
  }
  _place++;

  IceShaderBufferParameterFlags requiredParameters = 0;

  while (_place < _layout.length() && _layout[_place] != '}')
  {
    requiredParameters |= GetNextBufferParameter(_place, _layout);
  }

  return requiredParameters;
}

IceShaderBinding GetNextBinding(u64& _place,
                                std::string& _layout,
                                IceShaderBufferParameterFlags& _parameters)
{
  SkipWhiteSpace(_place, _layout);
  assert(_place < _layout.size());

  // Parse the file for bindings
  switch (_layout[_place])
  {
  case 'b':
  {
    _parameters = GetBufferParameters(_place, _layout);
    _place++;
    return Ice_Shader_Binding_Buffer;
  }
  case 's':
  {
    _place++;
    return Ice_Shader_Binding_Image;
  }
  }
  _place++;

  // End of file
  return Ice_Shader_Binding_Invalid;
}

void IvkMaterial_T::FillShaderBindings(IvkShader& _shader, const char* _directory)
{
  // Load the shader's layout file
  std::vector<char> layoutSource = FileSystem::LoadFile(_directory);
  std::string layout(layoutSource.data());

  IceShaderBinding bindInput;
  IceShaderBufferParameterFlags bufferParameters = 0;
  u64 place = 0;

  while ((bindInput = GetNextBinding(place, layout, info.bufferParameterFlags)) !=
         Ice_Shader_Binding_Invalid)
  {
    
    _shader.bindings.push_back(bindInput);
  }
}

// Transforms an Ice shader stage to a vulkan shader stage
VkShaderStageFlagBits ivkShaderStage(IceShaderStageFlags _stage)
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

void IvkMaterial_T::CreateDescriptorSetLayout(IceRenderContext* _rContext,
                                              const std::vector<IvkShader>& _shaders)
{
  u32 bindingIndex = 0;
  u32 imageCount = 0;
  std::vector<VkDescriptorSetLayoutBinding> stageBindings;
  VkDescriptorSetLayoutBinding binding{};
  binding.descriptorCount = 1;
  binding.pImmutableSamplers = nullptr;

  u32 nameint = 0;

  // Add shader bindings from all the program's shaders
  for (const auto& s : _shaders)
  {
    LogError("%s", info.sourceNames[nameint++]);

    binding.stageFlags = ivkShaderStage(s.stage);
    for (u32 i = 0; i < s.bindings.size(); i++)
    {
      switch (s.bindings[i])
      {
      case Ice_Shader_Binding_Buffer:
        LogDebug("Buffer");
        binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        break;
      case Ice_Shader_Binding_Image:
        LogDebug("Image");
        binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        imageCount++;
        break;
      }

      binding.binding = bindingIndex++;
      stageBindings.push_back(binding);
      info.bindings.push_back(s.bindings[i]);
    }
  }
  LogError("End shader descriptors");

  info.textures.resize(imageCount);

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

void IvkMaterial_T::CreateDescriptorSet(IceRenderContext* _rContext)
{
  VkDescriptorSetAllocateInfo allocInfo {};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = _rContext->descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &descriptorSetLayout;

  IVK_ASSERT(vkAllocateDescriptorSets(_rContext->device, &allocInfo, &descriptorSet),
             "Failed to allocate descriptor set");
}

void IvkMaterial_T::CreatePipelineLayout(IceRenderContext* _rContext)
{
  VkPushConstantRange mvpMatricesPushConstant;
  mvpMatricesPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  mvpMatricesPushConstant.size = 128; // model matrix (64) + view&projection matrix (64)
  mvpMatricesPushConstant.offset = 0;

  VkPipelineLayoutCreateInfo createInfo {};
  createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  createInfo.setLayoutCount = 1;
  createInfo.pSetLayouts = &descriptorSetLayout;
  createInfo.pushConstantRangeCount = 1;
  createInfo.pPushConstantRanges = &mvpMatricesPushConstant;

  IVK_ASSERT(vkCreatePipelineLayout(
                 _rContext->device, &createInfo, _rContext->allocator, &pipelineLayout),
             "Failed to create pipeline layout");
}

void IvkMaterial_T::CreatePipeline(IceRenderContext* _rContext, std::vector<IvkShader> _shaders)
{
  // Viewport State
  //=================================================
  // Defines the screen settings used during rasterization
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
  // Defines how vertex buffers are to be traversed
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
  // Defines how meshes are to be rendered
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo{};
  inputAssemblyStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssemblyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssemblyStateInfo.primitiveRestartEnable = VK_FALSE;

  // Rasterization State
  //=================================================
  // Defines how the pipeline will rasterize the image
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
  // States included here are capable of being set by dynamic state setting functions
  // When binding the pipeline, if these states are not set via one of the state setting functions
  //    the state's settings bound from the previous bound pipeline shall be used
  VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
  dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicStateInfo.dynamicStateCount = 0;
  dynamicStateInfo.pDynamicStates = nullptr;

  // Shader Stages State
  //=================================================
  // Insert shader modules
  std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfos(_shaders.size());

  for (u32 i = 0; i < _shaders.size(); i++)
  {
    shaderStageInfos[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfos[i].stage = ivkShaderStage(_shaders[i].stage);
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

