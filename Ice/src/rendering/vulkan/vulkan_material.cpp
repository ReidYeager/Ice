
#include "defines.h"

#include "rendering/vulkan/vulkan.h"
#include "core/platform/platform.h"
#include "tools/lexer.h"

#include <string>
#include <vector>

b8 Ice::RendererVulkan::CreateShader(Ice::ShaderSettings _settings, Ice::Shader* _shader)
{
  switch (_settings.type)
  {
  case Shader_Vertex: _settings.fileDirectory.append(".vert"); break;
  case Shader_Fragment: _settings.fileDirectory.append(".frag"); break;
  case Shader_Compute: _settings.fileDirectory.append(".comp"); break;
  default: IceLogError("Shader type unknown"); return {};
  }

  _shader->settings = _settings;

  ICE_ATTEMPT(CreateShaderModule(_shader));
  _shader->input = LoadShaderDescriptors(_shader);

  return true;
}

void Ice::RendererVulkan::DestroyShader(Ice::Shader& _shader)
{
  vkDeviceWaitIdle(context.device);
  vkDestroyShaderModule(context.device, _shader.vulkan.module, context.alloc);
}

b8 Ice::RendererVulkan::ReloadShader(Ice::Shader* _shader)
{
  vkDeviceWaitIdle(context.device);

  vkDestroyShaderModule(context.device, _shader->vulkan.module, context.alloc);

  _shader->input.clear();
  LoadShaderDescriptors(_shader);

  return CreateShaderModule(_shader);
}

b8 Ice::RendererVulkan::CreateShaderModule(Ice::Shader* _shader)
{
  std::string directory = _shader->settings.fileDirectory.c_str();
  directory.append(".spv");

  std::vector<char> source = Ice::LoadFile(directory.c_str());
  if (source.size() == 0)
    return false;

  VkShaderModuleCreateInfo createInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
  createInfo.codeSize = source.size();
  createInfo.pCode = reinterpret_cast<u32*>(source.data());

  IVK_ASSERT(vkCreateShaderModule(context.device,
                                  &createInfo,
                                  context.alloc,
                                  &_shader->vulkan.module),
             "failed to create shader module for %s", directory.c_str());

  return true;
}

std::vector<Ice::ShaderInputElement> Ice::RendererVulkan::LoadShaderDescriptors(Ice::Shader* _shader)
{
  // Descriptor loading could be useful across APIs, and most of the code will not change
  //  Should eventually look into abstracting this.

  std::string directory = _shader->settings.fileDirectory.c_str();
  directory.append(".desc");

  std::vector<char> descriptorSource = Ice::LoadFile(directory.c_str());
  if (descriptorSource.size() == 0)
    return {}; // Empty or non-existent file (No descriptors)

  Ice::Lexer lexer(descriptorSource);
  Ice::LexerToken token;

  Ice::ShaderInputElement newInput{};
  std::vector<ShaderInputElement> tmpInputs;

  while (!lexer.CompletedStream())
  {
    // Get descriptor information =====
    if (lexer.ExpectString("bindings") && lexer.ExpectString("{"))
    {
      while (!lexer.ExpectString("}"))
      {
        token = lexer.NextToken();

        // Check if the token is a valid descriptor
        u32 typeIndex = lexer.GetTokenSetIndex(token,
                                               Ice::ShaderInputTypeStrings,
                                               (u32)Ice::Shader_Input_Count,
                                               false);
        if (typeIndex == (u32)Ice::Shader_Input_Count)
        {
          IceLogWarning("Invalid descriptor '%s'\n> '%s'", token.string.c_str(), directory.c_str());
          continue;
        }
        newInput.type = (ShaderInputTypes)typeIndex;

        // Get input index
        if (!lexer.ExpectType(Ice::Token_Decimal, &token))
        {
          IceLogError("Binding '%s' is missing an index. Ignoring this binding.\n> '%s'",
                      token.string.c_str(),
                      _shader->settings.fileDirectory.c_str());
          continue;
        }
        newInput.inputIndex = lexer.GetUIntFromToken(&token);

        // Get optional input
        switch (newInput.type)
        {
        case Ice::Shader_Input_Buffer:
        {
          if (lexer.ExpectType(Ice::Token_Decimal, &token))
          {
            newInput.bufferSegment.elementSize = lexer.GetUIntFromToken(&token);
            newInput.bufferSegment.count = 1;
            newInput.bufferSegment.startIndex = newInput.inputIndex;
            if (newInput.bufferSegment.elementSize == 0)
            {
              IceLogError("Shader defines buffer descriptor of size 0\n> '%s'", directory.c_str());
            }
          }
          else
          {
            IceLogError("Shader not defining buffer descriptor size\n> '%s'", directory.c_str());
          }
        } break;
        default: break;
        }

        tmpInputs.push_back(newInput);
      }
    }
    // Get buffer information =====
    //else if (lexer.ExpectString("buffer") && lexer.ExpectString("{"))
    //{
    //  if (lexer.ExpectType(Token_Int, &token))
    //  {
    //    _shader->buffer.size = lexer.GetUIntFromToken(&token);
    //  }
    //}
    else
    {
      lexer.NextToken(); // Skip invalid token.
    }
  }

  return tmpInputs;
}

b8 Ice::RendererVulkan::CreateMaterial(Ice::Material* _material)
{
  ICE_ATTEMPT(CreateDescriptorLayoutAndSet(_material));

  // Define input resources =====
  {
    // Buffer =====
    if (_material->buffer.elementSize * _material->buffer.count)
    {
      CreateBufferMemory(&_material->buffer,
                         _material->buffer.elementSize,
                         _material->buffer.count,
                         Ice::Buffer_Memory_Shader_Read);

      // Associate the buffer segments with the material's buffer
      for (auto& d : _material->input)
      {
        if (d.type == Shader_Input_Buffer)
        {
          d.bufferSegment.buffer = &_material->buffer;
        }
      }
    }
  }

  UpdateDescriptorSet(&_material->vulkan.descriptorSet, _material->input);

  ICE_ATTEMPT(CreatePipelineLayout({ context.globalDescriptorLayout,
                                     context.cameraDescriptorLayout,
                                     _material->vulkan.descriptorSetLayout,
                                     context.objectDescriptorLayout },
                                   &_material->vulkan.pipelineLayout));
  ICE_ATTEMPT(CreatePipeline(_material));

  return true;
}

void Ice::RendererVulkan::DestroyMaterial(Ice::Material& _material)
{
  vkDeviceWaitIdle(context.device);

  //for (auto& s : _material.settings->shaders)
  //{
  //  DestroyBufferMemory(&s.buffer);
  //}
  DestroyBufferMemory(&_material.buffer);
  _material.shaders.clear();

  _material.input.clear();

  vkFreeDescriptorSets(context.device, context.descriptorPool, 1, &_material.vulkan.descriptorSet);
  vkDestroyDescriptorSetLayout(context.device, _material.vulkan.descriptorSetLayout, context.alloc);

  vkDestroyPipelineLayout(context.device, _material.vulkan.pipelineLayout, context.alloc);
  vkDestroyPipeline(context.device, _material.vulkan.pipeline, context.alloc);
}

b8 Ice::RendererVulkan::RecreateMaterial(Ice::Material* _material)
{
  vkDestroyPipeline(context.device, _material->vulkan.pipeline, context.alloc);
  vkDestroyPipelineLayout(context.device, _material->vulkan.pipelineLayout, context.alloc);
  vkFreeDescriptorSets(context.device, context.descriptorPool, 1, &_material->vulkan.descriptorSet);
  vkDestroyDescriptorSetLayout(context.device, _material->vulkan.descriptorSetLayout, context.alloc);

  _material->input.clear();

  return CreateMaterial(_material);
}

b8 Ice::RendererVulkan::AssembleMaterialDescriptorBindings(Ice::Material* _material,
                                                           std::vector<VkDescriptorSetLayoutBinding>& _bindings)
{
  u32 count = 0;
  std::vector<Ice::ShaderInputElement> orderedInputElements; // Used for faster place-checking

  // Count descriptors =====
  for (Ice::Shader* s : _material->shaders)
  {
    count += (u32)s->input.size();
  }
  _material->buffer.count = 1;
  _bindings.resize(count); // Worst-case size.
  orderedInputElements.resize(count);

  // Combine shader inputs =====
  u32 actualCount = 0;
  VkDescriptorSetLayoutBinding newBinding{};
  for (Ice::Shader* shader : _material->shaders)
  {
    for (auto& descriptor : shader->input)
    {
      if (orderedInputElements[descriptor.inputIndex].type == descriptor.type)
      {
        if (descriptor.type == Ice::Shader_Input_Buffer)
        {
          // The largest buffer contains the same data as all smaller buffers
          u64 max = max(orderedInputElements[descriptor.inputIndex].bufferSegment.elementSize,
                        descriptor.bufferSegment.elementSize);
          orderedInputElements[descriptor.inputIndex].bufferSegment.elementSize = max;
        }

        continue;
      }
      else if (orderedInputElements[descriptor.inputIndex].type != Ice::Shader_Input_Count)
      {
        IceLogFatal("Shader descriptor conflict at index %u", descriptor.inputIndex);
        _bindings.clear();
        return false;
      }

      newBinding.descriptorCount = 1;
      newBinding.pImmutableSamplers = nullptr;
      newBinding.binding = descriptor.inputIndex;
      newBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
      switch (descriptor.type)
      {
      case Ice::Shader_Input_Buffer:
      {
        newBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        // Create a new buffer element if this buffer binding index has not been encountered yet
        if (descriptor.bufferSegment.startIndex >= _material->buffer.count)
        {
          descriptor.bufferSegment.startIndex = _material->buffer.count;
          _material->buffer.count++;
          _material->buffer.elementSize = max(descriptor.bufferSegment.elementSize,
                                              _material->buffer.elementSize);
        }
      } break;
      case Ice::Shader_Input_Image2D:
      {
        newBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor.image = &defaultTexture;
      } break;
      default: continue;
      }

      orderedInputElements[descriptor.inputIndex] = descriptor;
      _bindings[descriptor.inputIndex] = newBinding;
      actualCount++;
    }
  }

  _material->input = orderedInputElements;
  _bindings.resize(actualCount);

  return true;
}

b8 Ice::RendererVulkan::CreateGlobalDescriptors()
{
  std::vector<VkDescriptorSetLayoutBinding> bindings;

  VkDescriptorSetLayoutBinding newBinding;
  newBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
  newBinding.pImmutableSamplers = nullptr;
  newBinding.descriptorCount = 1;

  // Global pipeline layout =====
  {
    // Global descriptors =====
    newBinding.binding = 0;
    newBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings.push_back(newBinding);

    ICE_ATTEMPT(CreateDescriptorLayoutAndSet(&bindings,
                                             &context.globalDescriptorLayout,
                                             &context.globalDescriptorSet));

    ICE_ATTEMPT(CreateBufferMemory(&context.globalDescriptorBuffer,
                                   64 + (2 * sizeof(Ice::vec4)),
                                   1,
                                   Ice::Buffer_Memory_Shader_Read));

    std::vector<Ice::ShaderInputElement> iceBind(1);
    iceBind[0].bufferSegment.buffer = &context.globalDescriptorBuffer;
    iceBind[0].bufferSegment.offset = 0;
    iceBind[0].bufferSegment.elementSize = 64;
    iceBind[0].type = Shader_Input_Buffer;
    iceBind[0].inputIndex = 0;
    UpdateDescriptorSet(&context.globalDescriptorSet, iceBind);

    // Camera descriptors =====
    bindings.clear();

    newBinding.binding = 0;
    newBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings.push_back(newBinding);

    ICE_ATTEMPT(CreateDescriptorLayout(&bindings, &context.cameraDescriptorLayout));

    // Pipeline =====
    ICE_ATTEMPT(CreatePipelineLayout({ context.globalDescriptorLayout,
                                       context.cameraDescriptorLayout },
                                     &context.globalPipelineLayout));
  }

  // Object descriptors =====
  bindings.clear();

  newBinding.binding = 0;
  newBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  bindings.push_back(newBinding);

  ICE_ATTEMPT(CreateDescriptorLayout(&bindings, &context.objectDescriptorLayout));

  return true;
}

void Ice::RendererVulkan::PushDataToGlobalDescriptors(void* _data, Ice::BufferSegment _segment /*= {}*/)
{
  _segment.buffer = &context.globalDescriptorBuffer;

  if (_segment.count == 0)
  {
    _segment.count = 1;
    _segment.elementSize = context.globalDescriptorBuffer.elementSize;
  }

  PushDataToBuffer(_data, _segment);
}

b8 Ice::RendererVulkan::CreateDescriptorLayoutAndSet(Ice::Material* _material)
{
  std::vector<VkDescriptorSetLayoutBinding> bindings;
  ICE_ATTEMPT(AssembleMaterialDescriptorBindings(_material, bindings));

  return CreateDescriptorLayoutAndSet(&bindings,
                                      &_material->vulkan.descriptorSetLayout,
                                      &_material->vulkan.descriptorSet);
}

b8 Ice::RendererVulkan::CreateDescriptorLayoutAndSet(std::vector<VkDescriptorSetLayoutBinding>* _bindings,
                                                     VkDescriptorSetLayout* _layout,
                                                     VkDescriptorSet* _set)
{
  ICE_ATTEMPT(CreateDescriptorLayout(_bindings, _layout));
  ICE_ATTEMPT(CreateDescriptorSet(_layout, _set));

  return true;
}

b8 Ice::RendererVulkan::CreateDescriptorLayout(std::vector<VkDescriptorSetLayoutBinding>* _bindings,
                                               VkDescriptorSetLayout* _layout)
{
  VkDescriptorSetLayoutCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  createInfo.flags = 0;
  createInfo.pNext = nullptr;
  createInfo.bindingCount = (u32)_bindings->size();
  createInfo.pBindings = _bindings->data();

  IVK_ASSERT(vkCreateDescriptorSetLayout(context.device,
                                         &createInfo,
                                         context.alloc,
                                         _layout),
             "Failed to create descriptor set layout");

  return true;
}

b8 Ice::RendererVulkan::CreateDescriptorSet(VkDescriptorSetLayout* _layout, VkDescriptorSet* _set)
{
  VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
  allocInfo.descriptorPool = context.descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = _layout;

  IVK_ASSERT(vkAllocateDescriptorSets(context.device, &allocInfo, _set),
             "Failed to allocate the descriptor set");

  return true;
}

void Ice::RendererVulkan::UpdateDescriptorSet(VkDescriptorSet* _set,
                                              const std::vector<Ice::ShaderInputElement>& _inputs)
{
  vkDeviceWaitIdle(context.device);

  std::vector<VkDescriptorImageInfo> images;
  images.reserve(_inputs.size());
  std::vector<VkDescriptorBufferInfo> buffers;
  buffers.reserve(_inputs.size());
  std::vector<VkWriteDescriptorSet> writes;

  VkDescriptorImageInfo newImage{};
  VkDescriptorBufferInfo newBuffer{};

  VkWriteDescriptorSet newWrite{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
  newWrite.dstSet = *_set;
  newWrite.dstArrayElement = 0;
  newWrite.descriptorCount = 1;
  newWrite.pTexelBufferView = nullptr;

  for (const auto& descriptor : _inputs)
  {
    newWrite.dstBinding = descriptor.inputIndex;

    switch (descriptor.type)
    {
    case Shader_Input_Buffer:
    {
      newBuffer.buffer = descriptor.bufferSegment.buffer->vulkan.buffer;
      newBuffer.offset = descriptor.bufferSegment.startIndex
        * descriptor.bufferSegment.buffer->padElementSize;
      newBuffer.range = descriptor.bufferSegment.elementSize;

      buffers.push_back(newBuffer);
      newWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      newWrite.pBufferInfo = &buffers[buffers.size() - 1];
      newWrite.pImageInfo = nullptr;
    } break;
    case Shader_Input_Image2D:
    {
      newImage.imageLayout = descriptor.image->vulkan.layout;
      newImage.imageView = descriptor.image->vulkan.view;
      newImage.sampler = descriptor.image->vulkan.sampler;

      images.push_back(newImage);
      newWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      newWrite.pBufferInfo = nullptr;
      newWrite.pImageInfo = &images[images.size() - 1];
    } break;
    default: continue;
    }

    writes.push_back(newWrite);
  }

  vkUpdateDescriptorSets(context.device, (u32)writes.size(), writes.data(), 0, nullptr);
}

b8 Ice::RendererVulkan::SetMaterialInput(Ice::Material* _material,
                                         u32 _bindIndex,
                                         Ice::Image* _image)
{
  Ice::ShaderInputElement imageInput;
  imageInput.inputIndex = _bindIndex;
  imageInput.type = Ice::Shader_Input_Image2D;
  imageInput.image = _image;
  std::vector<Ice::ShaderInputElement> inputs = { imageInput };

  UpdateDescriptorSet(&_material->vulkan.descriptorSet, inputs);
  return true;
}

VkVertexInputBindingDescription GetVertexBindingDescription()
{
  VkVertexInputBindingDescription desc = {};
  desc.stride = sizeof(Ice::Vertex);
  desc.binding = 0;
  desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  return desc;
}

std::vector<VkVertexInputAttributeDescription> GetVertexAttributeDescriptions()
{
  std::vector<VkVertexInputAttributeDescription> attribs(3);
  // Position
  attribs[0].binding = 0;
  attribs[0].location = 0;
  attribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  attribs[0].offset = offsetof(Ice::Vertex, position);
  // UV
  attribs[2].binding = 0;
  attribs[2].location = 1;
  attribs[2].format = VK_FORMAT_R32G32_SFLOAT;
  attribs[2].offset = offsetof(Ice::Vertex, uv);
  // normal
  attribs[1].binding = 0;
  attribs[1].location = 2;
  attribs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  attribs[1].offset = offsetof(Ice::Vertex, normal);

  return attribs;
}

b8 Ice::RendererVulkan::CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& _setLayouts,
                                             VkPipelineLayout* _pipelineLayout)
{
  VkPipelineLayoutCreateInfo createInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
  createInfo.flags = 0;
  createInfo.pNext = nullptr;
  createInfo.pushConstantRangeCount = 0;
  createInfo.pPushConstantRanges = nullptr;
  createInfo.setLayoutCount = (u32)_setLayouts.size();
  createInfo.pSetLayouts = _setLayouts.data();

  IVK_ASSERT(vkCreatePipelineLayout(context.device,
                                    &createInfo,
                                    context.alloc,
                                    _pipelineLayout),
             "Failed to create pipeline layout");

  return true;
}

b8 Ice::RendererVulkan::CreatePipeline(Ice::Material* _material)
{
  // Shader Stages State =====
  VkPipelineShaderStageCreateInfo shaderStage{};
  shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
  shaderStage.pName = "main";

  std::vector<VkPipelineShaderStageCreateInfo> stages(_material->shaders.size());
  for (u32 i = 0; i < _material->shaders.size(); i++)
  {
    shaderStage.module = _material->shaders[i]->vulkan.module;
    switch (_material->shaders[i]->settings.type)
    {
    case Shader_Vertex: shaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT; break;
    case Shader_Fragment: shaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
    default: return false;
    }

    stages[i] = shaderStage;
  }

  // Vertex Input State =====
  // Defines how vertex buffers are to be traversed
  const auto vertexInputAttribDesc = GetVertexAttributeDescriptions();
  const auto vertexInputBindingDesc = GetVertexBindingDescription();

  VkPipelineVertexInputStateCreateInfo vertexInputStateInfo{};
  vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  //vertexInputStateInfo.vertexAttributeDescriptionCount = 0;
  vertexInputStateInfo.vertexAttributeDescriptionCount = (u32)vertexInputAttribDesc.size();
  vertexInputStateInfo.pVertexAttributeDescriptions = vertexInputAttribDesc.data();
  //vertexInputStateInfo.vertexBindingDescriptionCount = 0;
  vertexInputStateInfo.vertexBindingDescriptionCount = 1;
  vertexInputStateInfo.pVertexBindingDescriptions = &vertexInputBindingDesc;

  // Input Assembly State =====
  // Defines how meshes are to be rendered
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo{};
  inputAssemblyStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssemblyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssemblyStateInfo.primitiveRestartEnable = VK_FALSE;

  // Viewport State =====
  // Defines the screen settings used during rasterization
  VkViewport viewport{};
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = (float)context.swapchainExtent.width;
  viewport.height = (float)context.swapchainExtent.height;
  viewport.minDepth = 0;
  viewport.maxDepth = 1;

  VkRect2D scissor{};
  scissor.extent = context.swapchainExtent;
  scissor.offset = { 0, 0 };

  VkPipelineViewportStateCreateInfo viewportStateInfo{};
  viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportStateInfo.viewportCount = 1;
  viewportStateInfo.pViewports = &viewport;
  viewportStateInfo.scissorCount = 1;
  viewportStateInfo.pScissors = &scissor;

  // Rasterization State =====
  // Defines how the pipeline will rasterize the image
  VkPipelineRasterizationStateCreateInfo rasterStateInfo{};
  rasterStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
  //rasterStateInfo.polygonMode = VK_POLYGON_MODE_LINE;
  rasterStateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterStateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
  //rasterStateInfo.cullMode = VK_CULL_MODE_NONE;

  rasterStateInfo.rasterizerDiscardEnable = VK_TRUE;
  rasterStateInfo.lineWidth = 1.0f;
  rasterStateInfo.depthBiasEnable = VK_FALSE;
  rasterStateInfo.depthClampEnable = VK_FALSE;
  rasterStateInfo.rasterizerDiscardEnable = VK_FALSE;

  // Multisampling State =====
  VkPipelineMultisampleStateCreateInfo multisampleStateInfo{};
  multisampleStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampleStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampleStateInfo.sampleShadingEnable = VK_FALSE;

  // Depth Stencil State =====
  VkPipelineDepthStencilStateCreateInfo depthStateInfo{};
  depthStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStateInfo.depthTestEnable = VK_TRUE;
  depthStateInfo.depthWriteEnable = VK_TRUE;
  depthStateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
  depthStateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
  depthStateInfo.depthBoundsTestEnable = VK_FALSE;

  // Color Blend State =====
  VkPipelineColorBlendAttachmentState blendAttachmentStates[4]{};
  blendAttachmentStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT
    | VK_COLOR_COMPONENT_G_BIT
    | VK_COLOR_COMPONENT_B_BIT
    | VK_COLOR_COMPONENT_A_BIT;
  blendAttachmentStates[0].blendEnable = VK_FALSE;
  blendAttachmentStates[1] = blendAttachmentStates[0];
  blendAttachmentStates[2] = blendAttachmentStates[0];
  blendAttachmentStates[3] = blendAttachmentStates[0];

  VkPipelineColorBlendStateCreateInfo blendStateInfo{};
  blendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  blendStateInfo.logicOpEnable = VK_FALSE;
  blendStateInfo.attachmentCount = 1;
  blendStateInfo.pAttachments = &blendAttachmentStates[0];

  // Dynamic States =====
  // States included here are capable of being set by dynamic state setting functions
  // When binding the pipeline, if these states are not set via one of the state setting functions
  //    the state's settings bound from the previous bound pipeline shall be used
  VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
  dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicStateInfo.dynamicStateCount = 0;
  dynamicStateInfo.pDynamicStates = nullptr;

  // Creation =====
  VkGraphicsPipelineCreateInfo createInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
  createInfo.pViewportState = &viewportStateInfo;
  createInfo.pVertexInputState = &vertexInputStateInfo;
  createInfo.pInputAssemblyState = &inputAssemblyStateInfo;
  createInfo.pRasterizationState = &rasterStateInfo;
  createInfo.pMultisampleState = &multisampleStateInfo;
  createInfo.pDepthStencilState = &depthStateInfo;
  createInfo.pColorBlendState = &blendStateInfo;
  createInfo.pDynamicState = &dynamicStateInfo;
  createInfo.stageCount = (u32)stages.size();
  createInfo.pStages = stages.data();

  createInfo.layout = _material->vulkan.pipelineLayout;
  createInfo.renderPass = context.forward.renderpass;
  createInfo.subpass = _material->subpassIndex;

  IVK_ASSERT(vkCreateGraphicsPipelines(context.device,
                                       nullptr,
                                       1,
                                       &createInfo,
                                       context.alloc,
                                       &_material->vulkan.pipeline),
             "Failed to create the pipeline");

  return true;
}
