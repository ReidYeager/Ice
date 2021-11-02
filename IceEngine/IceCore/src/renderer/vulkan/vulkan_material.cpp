
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
                               std::vector<iceImage_t*> _images /*= {}*/)
{
  IceLogDebug("Creating material");
  info.sourceNames = _shaderNames;
  info.sourceStages = _shaderStages;

  // Load the shaders and fill their bindings
  shaders = GetShaders(_rContext);

  // Define descriptor set
  CreateDescriptorSetLayout(_rContext, shaders);
  CreateDescriptorSets(_rContext);

  // Binds any included images, and updates the descriptor sets
  UpdateImages(_rContext, _images);

  CreatePipelineComponents(_rContext);

  IceLogDebug("Material created");
}

void IvkMaterial_T::Shutdown(IceRenderContext* _rContext)
{
  IceLogDebug("Destroying material");

  DestroyPipelineComponents(_rContext);

  // NOTE : This does not remove descriptor sets from the descriptor pool
  //        If materials are re-loaded enough times, the max-sets will be reached
  //        This will trigger a breakpoint at "vkAllocateDescriptorSets" on material re-creation
  for (VkDescriptorSetLayout& dsLayout : dSetLayouts)
  {
    vkDestroyDescriptorSetLayout(_rContext->device, dsLayout, _rContext->allocator);
  }

  // Clear shader modules
  for (IvkShader s : shaders)
  {
    vkDestroyShaderModule(_rContext->device, s.module, _rContext->allocator);
  }
  shaders.clear();

  for (IceBuffer& buffer : shaderBuffers)
  {
    buffer->Free(_rContext);
    delete(buffer);
  }
  shaderBuffers.clear();
}

void IvkMaterial_T::DestroyPipelineComponents(IceRenderContext* _rContext)
{
  vkDestroyPipeline(_rContext->device, pipeline, _rContext->allocator);
  vkDestroyPipelineLayout(_rContext->device, pipelineLayout, _rContext->allocator);
}

void IvkMaterial_T::CreatePipelineComponents(IceRenderContext* _rContext)
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
      IceLogError("Shader stage %u not recognized", info.sourceStages[i]);
    }

    // If the time modified is more recent than time loaded
    struct _stat result;
    if (_stat(shaderDir.c_str(), &result) == 0 &&
        result.st_mtime != info.sourceLastModifiedTimes[i])
    {
      IceLogInfo("Shader --- %20s --- loaded: %u, new: %u",
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

    Shutdown(_rContext);
    Initialize(_rContext, info.sourceNames, info.sourceStages, info.textures);
  }
}

void IvkMaterial_T::UpdateBuffer(IceRenderContext* _rContext,
                                 IceShaderStageFlags _stage,
                                 IceShaderBufferParameterFlags _userParameterFlags,
                                 void* _userData)
{
  u32 i;
  for (i = 0; i < 3; i++)
  {
    if ((1 << i) == _stage)
      break;
  }

  IceShaderBufferParameterFlags params = shaders[i].bufferParameters;
  i = shaders[i].bufferIndex;

  if (i >= shaderBuffers.size())
    return;

  // Update user data
  if (_userData != nullptr)
  {
    // Update selected user parameters
    FillShaderBuffer(_rContext, shaderBuffers[i], _userData, _userParameterFlags, params);
  }
}

void IvkMaterial_T::UpdateImages(IceRenderContext* _rContext,
                                  std::vector<iceImage_t*> _images)
{
  u64 imageCount = (_images.size() < info.textures.size()) ? _images.size() : info.textures.size();
  u64 bufferCount = shaderBuffers.size();
  std::vector<VkWriteDescriptorSet> writeSets(imageCount + bufferCount);
  std::vector<VkDescriptorBufferInfo> bufferInfos(bufferCount);
  std::vector<VkDescriptorImageInfo> imageInfos(imageCount);

  u32 bufferIndex = 0;
  u32 writeIndex = 0;
  u32 bindIndex = 0;
  u32 imageIndex = 0;

  // TODO : ~!!~ Push renderer information to the respective parameters
  //        As a test: send the VP matrix via only parameters

  u32 setIndex = 0;
  for (IvkShader const& shader : shaders)
  {
    bindIndex = 0;

    for (IceShaderBinding bindingType : shader.bindings)
    {
      switch (bindingType)
      {
      case Ice_Shader_Binding_Buffer:
      {
        bufferInfos[bufferIndex].buffer = shaderBuffers[shader.bufferIndex]->GetBuffer();
        bufferInfos[bufferIndex].offset = 0;
        bufferInfos[bufferIndex].range = VK_WHOLE_SIZE;

        writeSets[writeIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeSets[writeIndex].dstSet = descriptorSets[setIndex];
        writeSets[writeIndex].dstBinding = bindIndex;
        writeSets[writeIndex].dstArrayElement = 0;
        writeSets[writeIndex].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeSets[writeIndex].descriptorCount = 1;
        writeSets[writeIndex].pBufferInfo = &bufferInfos[bufferIndex];
        writeSets[writeIndex].pImageInfo = nullptr;
        writeSets[writeIndex].pTexelBufferView = nullptr;

        bufferIndex++;
        break;
      }
      case Ice_Shader_Binding_Image:
      {
        if (_images.size() == 0)
          break;

        info.textures[imageIndex] = _images[imageIndex];
        imageInfos[imageIndex].imageLayout = info.textures[imageIndex]->layout;
        imageInfos[imageIndex].imageView = info.textures[imageIndex]->view;
        imageInfos[imageIndex].sampler = info.textures[imageIndex]->sampler;

        writeSets[writeIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeSets[writeIndex].dstSet = descriptorSets[setIndex];
        writeSets[writeIndex].dstBinding = bindIndex;
        writeSets[writeIndex].dstArrayElement = 0;
        writeSets[writeIndex].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeSets[writeIndex].descriptorCount = 1;
        writeSets[writeIndex].pBufferInfo = nullptr;
        writeSets[writeIndex].pImageInfo = &imageInfos[imageIndex];
        writeSets[writeIndex].pTexelBufferView = nullptr;

        imageIndex++;
        break;
      }
      default:
        break;
      }

      bindIndex++;
      writeIndex++;
    }

    setIndex++;
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
                          descriptorSets.size(),
                          descriptorSets.data(),
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
    IceLogInfo("Shader : %s", info.sourceNames[i]);
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
    IceLogError("Shader stage %u not recognized", _stage);
    return {0, VK_NULL_HANDLE, {}};
  }

  IceLogInfo("Load Shader : %s", shaderDir.c_str());

  // Load and process the shader and its bindings
  CreateShaderModule(_rContext, shader, shaderDir.c_str());
  FillShaderBindings(shader, layoutDir.c_str());
  shader.stage = _stage;

  // Create buffer based on required buffer parameters
  u64 size;
  u64 RequiredBuffers = shader.bufferParameters;
  // Count the number of set buffer parameter flags
  for (size = 0; RequiredBuffers; size++)
  {
    RequiredBuffers &= RequiredBuffers - 1;
  }

  shader.bufferIndex = -1;
  if (size > 0)
  {
    IceBuffer sb = new IvkBuffer(_rContext,
                                 size * 16,
                                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                                    VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    shader.bufferIndex = (u32)shaderBuffers.size();
    shaderBuffers.push_back(sb);
  }

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

IceShaderBufferParameterFlags StringToParam(b8 _isBuffer, const char* _str)
{
  #define STP(string, param)                    \
  {                                             \
    if (std::strcmp(string, _str) == 0)  \
    return param; \
  }

  if (_isBuffer)
  {
    STP("ModelMatrix", Ice_Shader_Buffer_Param_ModelMatrix);
    STP("VpMatrix", Ice_Shader_Buffer_Param_VpMatrix);
    STP("ViewMatrix", Ice_Shader_Buffer_Param_ViewMatrix);
    STP("ProjectionMatrix", Ice_Shader_Buffer_Param_ProjectionMatrix);

    STP("User0", Ice_Shader_Buffer_Param_User0);
    STP("User1", Ice_Shader_Buffer_Param_User1);
    STP("User2", Ice_Shader_Buffer_Param_User2);
    STP("User3", Ice_Shader_Buffer_Param_User3);
    STP("User4", Ice_Shader_Buffer_Param_User4);
    STP("User5", Ice_Shader_Buffer_Param_User5);
    STP("User6", Ice_Shader_Buffer_Param_User6);
    STP("User7", Ice_Shader_Buffer_Param_User7);
    STP("User8", Ice_Shader_Buffer_Param_User8);
    STP("User9", Ice_Shader_Buffer_Param_User9);
    STP("User10", Ice_Shader_Buffer_Param_User10);
    STP("User11", Ice_Shader_Buffer_Param_User11);
  }
  else
  {
    STP("DepthImage", Ice_Shader_Image_Param_DepthImage);
    STP("User0", Ice_Shader_Image_Param_User0);
    STP("User1", Ice_Shader_Image_Param_User1);
    STP("User2", Ice_Shader_Image_Param_User2);
    STP("User3", Ice_Shader_Image_Param_User3);
    STP("User4", Ice_Shader_Image_Param_User4);
    STP("User5", Ice_Shader_Image_Param_User5);
    STP("User6", Ice_Shader_Image_Param_User6);
    STP("User7", Ice_Shader_Image_Param_User7);
    STP("User8", Ice_Shader_Image_Param_User8);
    STP("User9", Ice_Shader_Image_Param_User9);
  }

  #undef STP

  return 0;
}

IceShaderBufferParameterFlags GetNextBufferParameter(b8 _isBuffer,
                                                     u64& _place,
                                                     std::string& _layout)
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

  return StringToParam(_isBuffer, _layout.substr(start, _place - start).c_str());
}

IceShaderBufferParameterFlags GetShaderParameters(b8 _isBuffer, u64& _place, std::string& _layout, std::vector<IceShaderBinding>& _bindings)
{
  while (_place < _layout.length() && _layout[_place] != '{')
  {
    _place++;
  }
  _place++;

  IceShaderBufferParameterFlags requiredParameters = 0;

  while (_place < _layout.length() && _layout[_place] != '}')
  {
    IceShaderBufferParameterFlags newParam = GetNextBufferParameter(_isBuffer, _place, _layout);
    requiredParameters |= newParam;

    if (newParam)
    {
      _bindings.push_back((_isBuffer ? Ice_Shader_Binding_Buffer : Ice_Shader_Binding_Image));
    }
  }

  return requiredParameters;
}

IceShaderBinding GetNextBinding(u64& _place,
                                std::string& _layout,
                                std::vector<IceShaderBinding>& _bindings,
                                IceShaderBufferParameterFlags& _bufferParameters,
                                IceShaderImageParameterFlags& _imageParameters)
{
  SkipWhiteSpace(_place, _layout);
  assert(_place < _layout.size());

  // Parse the file for bindings
  switch (_layout[_place])
  {
  case 'b':
  {
    _bufferParameters = GetShaderParameters(true, _place, _layout, _bindings);
    _place++;
    return Ice_Shader_Binding_Buffer;
  }
  case 's':
  {
    _imageParameters = GetShaderParameters(false, _place, _layout, _bindings);
    _place++;
    return Ice_Shader_Binding_Image;
  }
  //case 'p':
  //{
  //  _bufferParameters = GetShaderParameters(true, _place, _layout); // Replace with pushParameters
  //  _place++;
  //  return Ice_Shader_Binding_PushConstant;
  //}
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

  while ((bindInput = GetNextBinding(place,
                                     layout,
                                     _shader.bindings,
                                     _shader.bufferParameters,
                                     _shader.imageParameters)) !=
         Ice_Shader_Binding_Invalid)
  {
    info.bufferParameterFlags |= _shader.bufferParameters;
    info.imageParameterFlags |= _shader.imageParameters;
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
  std::vector<std::vector<VkDescriptorSetLayoutBinding>> shaderBindings(info.sourceNames.size());
  VkDescriptorSetLayoutBinding binding{};
  binding.descriptorCount = 1;
  binding.pImmutableSamplers = nullptr;

  u32 shaderIndex = 0;

  std::vector<VkDescriptorSetLayoutCreateInfo> createInfos(info.sourceNames.size());
  dSetLayouts.resize(info.sourceNames.size());

  // Add shader bindings from all the program's shaders
  for (const auto& shader : _shaders)
  {
    IceLogDebug("%s", info.sourceNames[shaderIndex]);
    u32 shaderBufferBindings = 0;
    bindingIndex = 0;
    //shaderBindings[shaderIndex].clear();

    std::vector<VkDescriptorSetLayoutBinding>& shaderBinding = shaderBindings[shaderIndex];

    binding.stageFlags = ivkShaderStage(shader.stage);
    for (u32 i = 0; i < shader.bindings.size(); i++)
    {
      switch (shader.bindings[i])
      {
      case Ice_Shader_Binding_Buffer:
        IceLogInfo("Buffer");
        binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        break;
      case Ice_Shader_Binding_Image:
        IceLogInfo("Image");
        binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        imageCount++;
        break;
      }

      binding.binding = bindingIndex++;
      shaderBinding.push_back(binding);
      info.bindings.push_back(shader.bindings[i]);
    }

    createInfos[shaderIndex].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfos[shaderIndex].bindingCount = static_cast<uint32_t>(shaderBinding.size());
    createInfos[shaderIndex].pBindings = shaderBinding.data();

    IVK_ASSERT(vkCreateDescriptorSetLayout(_rContext->device,
                                           &createInfos[shaderIndex],
                                           _rContext->allocator,
                                           &dSetLayouts[shaderIndex]),
                                           "Failed to create descriptor set layout");

    shaderIndex++;
  }
  info.textures.resize(imageCount);
}

void IvkMaterial_T::CreateDescriptorSets(IceRenderContext* _rContext)
{
  VkDescriptorSetAllocateInfo allocInfo {};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = _rContext->descriptorPool;
  allocInfo.descriptorSetCount = dSetLayouts.size(); // Create a set for each shader
  allocInfo.pSetLayouts = dSetLayouts.data();

  descriptorSets.resize(dSetLayouts.size());

  IVK_ASSERT(vkAllocateDescriptorSets(_rContext->device, &allocInfo, descriptorSets.data()),
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
  createInfo.setLayoutCount = dSetLayouts.size();
  createInfo.pSetLayouts = dSetLayouts.data();
  createInfo.pushConstantRangeCount = 1;
  createInfo.pPushConstantRanges = &mvpMatricesPushConstant;

  IVK_ASSERT(vkCreatePipelineLayout(_rContext->device,
                                    &createInfo,
                                    _rContext->allocator,
                                    &pipelineLayout),
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

