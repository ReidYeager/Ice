
#include "defines.h"

#include "rendering/vulkan/vulkan.h"
#include "platform/platform.h"
#include "tools/lexer.h"

#include <string>
#include <vector>

b8 Ice::RendererVulkan::CreateShader(Ice::Shader* _shader)
{
  Ice::Shader& newShader = *_shader;

  newShader.bufferComponents = 0;
  newShader.fileDirectory = std::string(ICE_RESOURCE_SHADER_DIR).append(newShader.fileDirectory);
  //newShader.fileDirectory.append(newShader.fileDirectory.c_str());
  switch (newShader.type)
  {
  case Shader_Vertex: newShader.fileDirectory.append(".vert"); break;
  case Shader_Fragment: newShader.fileDirectory.append(".frag"); break;
  case Shader_Compute: newShader.fileDirectory.append(".comp"); break;
  default: IceLogError("Shader type unknown"); return {};
  }

  ICE_ATTEMPT(CreateShaderModule(&newShader));
  LoadShaderDescriptors(&newShader);

  return true;
}

void Ice::RendererVulkan::DestroyShader(Ice::Shader& _shader)
{
  vkDeviceWaitIdle(context.device);
  vkDestroyShaderModule(context.device, _shader.ivkShaderModule, context.alloc);
}

b8 Ice::RendererVulkan::CreateShaderModule(Ice::Shader* _shader)
{
  std::string directory = _shader->fileDirectory.c_str();
  directory.append(".spv");

  std::vector<char> source = Ice::LoadFile(directory.c_str());
  if (source.size() == 0)
    return false;

  VkShaderModuleCreateInfo createInfo { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
  createInfo.codeSize = source.size();
  createInfo.pCode = reinterpret_cast<u32*>(source.data());

  IVK_ASSERT(vkCreateShaderModule(context.device,
                                  &createInfo,
                                  context.alloc,
                                  &_shader->ivkShaderModule),
             "failed to create shader module for %s", directory.c_str());

  return true;
}

void Ice::RendererVulkan::LoadShaderDescriptors(Ice::Shader* _shader)
{
  // NOTE : Descriptor loading could be useful across APIs, and most of the code will not change
  //  Should eventually look into abstracting this.

  std::string directory = _shader->fileDirectory.c_str();
  directory.append(".desc");

  std::vector<char> descriptorSource = Ice::LoadFile(directory.c_str());
  if (descriptorSource.size() == 0)
    return; // Empty or non-existent file (No descriptors)

  Ice::Lexer lexer(descriptorSource);
  Ice::LexerToken token;

  Ice::ShaderInputElement newInput {};

  while (!lexer.CompletedStream())
  {
    // Get buffer information =====
    if (lexer.ExpectString("buffer") && lexer.ExpectString("{"))
    {
      while (!lexer.ExpectString("}"))
      {
        token = lexer.NextToken();

        // Check if the token is a valid buffer component
        u32 typeIndex = lexer.GetTokenSetIndex(token,
                                               Ice::ShaderBufferComponentStrings,
                                               (u32)Ice::Shader_Buffer_Count);
        if (typeIndex == (u32)Ice::Shader_Buffer_Count)
        {
          IceLogWarning("Ivalid buffer component '%s'\n> '%s'",
                        token.string.c_str(),
                        directory.c_str());
          continue;
        }

        _shader->bufferComponents |= (1 << typeIndex);
      }
    }

    // Get descriptor information =====
    if (lexer.ExpectString("bindings") && lexer.ExpectString("{"))
    {
      while (!lexer.ExpectString("}"))
      {
        token = lexer.NextToken();

        // Check if the token is a valid descriptor
        u32 typeIndex = lexer.GetTokenSetIndex(token,
                                               Ice::ShaderInputTypeStrings,
                                               (u32)Ice::Shader_Input_Count);
        if (typeIndex == (u32)Ice::Shader_Input_Count)
        {
          IceLogWarning("Ivalid descriptor '%s'\n> '%s'", token.string.c_str(), directory.c_str());
          continue;
        }

        if (!lexer.ExpectType(Ice::Token_Int, &token))
        {
          IceLogError("Binding '%s' is missing an index. Ignoring this binding.\n> '%s'",
                      token.string.c_str(),
                      _shader->fileDirectory.c_str());
          continue;
        }

        // Include the descriptor
        newInput.type = (ShaderInputTypes)typeIndex;
        newInput.inputIndex = lexer.GetUIntFromToken(&token);
        _shader->input.push_back(newInput);
      }
    }
  }

}

b8 Ice::RendererVulkan::CreateMaterial(Ice::Material* _material)
{
  ICE_ATTEMPT(CreateDescriptorLayoutAndSet(_material));

  // TODO : Assign default descriptor values (new buffer, default textures)

  ICE_ATTEMPT(CreatePipelineLayout({ context.globalDescriptorLayout,
                                     _material->ivkDescriptorSetLayout },
                                   &_material->ivkPipelineLayout));
  ICE_ATTEMPT(CreatePipeline(_material));

  return true;
}

void Ice::RendererVulkan::DestroyMaterial(Ice::Material& _material)
{
  vkDeviceWaitIdle(context.device);

  vkFreeDescriptorSets(context.device, context.descriptorPool, 1, &_material.ivkDescriptorSet);
  vkDestroyDescriptorSetLayout(context.device, _material.ivkDescriptorSetLayout, context.alloc);

  vkDestroyPipelineLayout(context.device, _material.ivkPipelineLayout, context.alloc);
  vkDestroyPipeline(context.device, _material.ivkPipeline, context.alloc);
}

b8 AssembleMaterialDescriptorBindings(Ice::Material* _material,
                                      std::vector<VkDescriptorSetLayoutBinding>& _bindings)
{
  // TODO : Need to account for descriptor binding gaps

  u32 count = 0;
  std::vector<Ice::ShaderInputElement> orderedInputElements; // Used for faster place-checking

  std::vector<Ice::Shader>& shaders = _material->settings->shaders;

  // Collect descriptors =====
  for (const Ice::Shader& s : shaders)
  {
    count += s.input.size();
  }
  _bindings.resize(count); // Worst-case size.
  orderedInputElements.resize(count);

  u32 actualCount = 0;
  VkDescriptorSetLayoutBinding newBinding {};
  for (const Ice::Shader& shader : shaders)
  {
    _material->settings->bufferComponents |= shader.bufferComponents;

    for (const auto& descriptor : shader.input)
    {
      if (orderedInputElements[descriptor.inputIndex].type == descriptor.type)
      {
        continue;
      }
      else if (orderedInputElements[descriptor.inputIndex].type != Ice::Shader_Input_Count)
      {
        IceLogError("Shader descriptor conflict at index %u", descriptor.inputIndex);
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
      } break;
      case Ice::Shader_Input_Image:
      {
        newBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      } break;
      default: continue;
      }

      _bindings[descriptor.inputIndex] = newBinding;
      orderedInputElements[descriptor.inputIndex] = descriptor;
      actualCount++;
    }
  }

  _material->settings->input = orderedInputElements;
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

  newBinding.binding = 0;
  newBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  bindings.push_back(newBinding);

  ICE_ATTEMPT(CreateDescriptorLayoutAndSet(&bindings,
                                           &context.globalDescriptorLayout,
                                           &context.globalDescriptorSet));

  ICE_ATTEMPT(CreatePipelineLayout({context.globalDescriptorLayout}, &context.globalPipelineLayout));

  ICE_ATTEMPT(CreateBufferMemory(&context.globalDescriptorBuffer,
                                 64,
                                 Ice::Buffer_Memory_Shader_Read));

  Ice::BufferSegment buffer {};
  buffer.offset = 0;
  buffer.size = 64;
  buffer.ivkBuffer = context.globalDescriptorBuffer.ivkBuffer;
  Ice::ShaderInputElement iceBind {};
  iceBind.assignedData = &buffer;
  iceBind.type = Shader_Input_Buffer;
  iceBind.inputIndex = 0;
  UpdateDescriptorSet(context.globalDescriptorSet, {iceBind});

  return true;
}

b8 Ice::RendererVulkan::CreateDescriptorLayoutAndSet(Ice::Material* _material)
{
  std::vector<VkDescriptorSetLayoutBinding> bindings;
  ICE_ATTEMPT(AssembleMaterialDescriptorBindings(_material, bindings));

  return CreateDescriptorLayoutAndSet(&bindings,
                                      &_material->ivkDescriptorSetLayout,
                                      &_material->ivkDescriptorSet);
}

b8 Ice::RendererVulkan::CreateDescriptorLayoutAndSet(std::vector<VkDescriptorSetLayoutBinding>* _bindings,
                                                     VkDescriptorSetLayout* _layout,
                                                     VkDescriptorSet* _set)
{
  // Create layout =====
  VkDescriptorSetLayoutCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  createInfo.flags = 0;
  createInfo.pNext = nullptr;
  createInfo.bindingCount = _bindings->size();
  createInfo.pBindings = _bindings->data();

  IVK_ASSERT(vkCreateDescriptorSetLayout(context.device,
             &createInfo,
             context.alloc,
             _layout),
             "Failed to create descriptor set layout");

  // Create set =====
  VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
  allocInfo.descriptorPool = context.descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = _layout;

  IVK_ASSERT(vkAllocateDescriptorSets(context.device, &allocInfo, _set),
             "Failed to allocate the descriptor set");

  return true;
}

void Ice::RendererVulkan::UpdateDescriptorSet(VkDescriptorSet& _set,
                                              std::vector<Ice::ShaderInputElement> _bindings)
{
  std::vector<VkDescriptorImageInfo> images;
  images.reserve(_bindings.size());
  std::vector<VkDescriptorBufferInfo> buffers;
  buffers.reserve(_bindings.size());
  std::vector<VkWriteDescriptorSet> writes;

  VkDescriptorImageInfo newImage {};
  VkDescriptorBufferInfo newBuffer {};

  VkWriteDescriptorSet newWrite { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
  newWrite.dstSet = _set;
  newWrite.dstArrayElement = 0;
  newWrite.descriptorCount = 1;
  newWrite.pTexelBufferView = nullptr;

  for (const auto& b : _bindings)
  {
    newWrite.dstBinding = b.inputIndex;

    switch (b.type)
    {
    case Shader_Input_Buffer:
    {
      newBuffer.buffer = ((Ice::BufferSegment*)b.assignedData)->ivkBuffer;
      newBuffer.offset = ((Ice::BufferSegment*)b.assignedData)->offset;
      newBuffer.range = ((Ice::BufferSegment*)b.assignedData)->size;

      buffers.push_back(newBuffer);
      newWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      newWrite.pBufferInfo = &buffers[buffers.size() - 1];
      newWrite.pImageInfo = nullptr;
    } break;
    case Shader_Input_Image:
    {
      newImage.imageLayout = ((Ice::IvkImage*)b.assignedData)->layout;
      newImage.imageView = ((Ice::IvkImage*)b.assignedData)->view;
      newImage.sampler = ((Ice::IvkImage*)b.assignedData)->sampler;

      images.push_back(newImage);
      newWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      newWrite.pBufferInfo = nullptr;
      newWrite.pImageInfo = &images[images.size() - 1];
    } break;
    }

    writes.push_back(newWrite);
  }

  vkUpdateDescriptorSets(context.device, writes.size(), writes.data(), 0, nullptr);
}

b8 Ice::RendererVulkan::CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& _setLayouts,
                                             VkPipelineLayout* _pipelineLayout)
{
  VkPipelineLayoutCreateInfo createInfo { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
  createInfo.flags = 0;
  createInfo.pNext = nullptr;
  createInfo.pushConstantRangeCount = 0;
  createInfo.pPushConstantRanges = nullptr;
  createInfo.setLayoutCount = _setLayouts.size();
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
  VkPipelineShaderStageCreateInfo shaderStage {};
  shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
  shaderStage.pName = "main";

  std::vector<VkPipelineShaderStageCreateInfo> stages(_material->settings->shaders.size());
  for (u32 i = 0; i < _material->settings->shaders.size(); i++)
  {
    shaderStage.module = _material->settings->shaders[i].ivkShaderModule;
    switch (_material->settings->shaders[i].type)
    {
    case Shader_Vertex: shaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT; break;
    case Shader_Fragment: shaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
    default: return false;
    }

    stages[i] = shaderStage;
  }

  // Vertex Input State =====
  // Defines how vertex buffers are to be traversed
  const auto vertexInputAttribDesc = Ice::IvkVertex::GetAttributeDescriptions();
  const auto vertexInputBindingDesc = Ice::IvkVertex::GetBindingDescription();

  VkPipelineVertexInputStateCreateInfo vertexInputStateInfo {};
  vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  //vertexInputStateInfo.vertexAttributeDescriptionCount = 0;
  vertexInputStateInfo.vertexAttributeDescriptionCount = vertexInputAttribDesc.size();
  vertexInputStateInfo.pVertexAttributeDescriptions    = vertexInputAttribDesc.data();
  //vertexInputStateInfo.vertexBindingDescriptionCount = 0;
  vertexInputStateInfo.vertexBindingDescriptionCount = 1;
  vertexInputStateInfo.pVertexBindingDescriptions    = &vertexInputBindingDesc;

  // Input Assembly State =====
  // Defines how meshes are to be rendered
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo {};
  inputAssemblyStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssemblyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssemblyStateInfo.primitiveRestartEnable = VK_FALSE;

  // Viewport State =====
  // Defines the screen settings used during rasterization
  VkViewport viewport {};
  viewport.x = 0;
  viewport.y = 0;
  viewport.width  = (float)context.swapchainExtent.width;
  viewport.height = (float)context.swapchainExtent.height;
  viewport.minDepth = 0;
  viewport.maxDepth = 1;

  VkRect2D scissor{};
  scissor.extent = context.swapchainExtent;
  scissor.offset = { 0, 0 };

  VkPipelineViewportStateCreateInfo viewportStateInfo {};
  viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportStateInfo.viewportCount = 1;
  viewportStateInfo.pViewports = &viewport;
  viewportStateInfo.scissorCount = 1;
  viewportStateInfo.pScissors = &scissor;

  // Rasterization State =====
  // Defines how the pipeline will rasterize the image
  VkPipelineRasterizationStateCreateInfo rasterStateInfo {};
  rasterStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  //rasterStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
  rasterStateInfo.polygonMode = VK_POLYGON_MODE_LINE;
  rasterStateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  //rasterStateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
  //rasterStateInfo.cullMode = VK_CULL_MODE_NONE;

  rasterStateInfo.rasterizerDiscardEnable = VK_TRUE;
  rasterStateInfo.lineWidth = 1.0f;
  rasterStateInfo.depthBiasEnable = VK_FALSE;
  rasterStateInfo.depthClampEnable = VK_FALSE;
  rasterStateInfo.rasterizerDiscardEnable = VK_FALSE;

  // Multisampling State =====
  VkPipelineMultisampleStateCreateInfo multisampleStateInfo {};
  multisampleStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampleStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampleStateInfo.sampleShadingEnable = VK_FALSE;

  // Depth Stencil State =====
  VkPipelineDepthStencilStateCreateInfo depthStateInfo {};
  depthStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStateInfo.depthTestEnable = VK_TRUE;
  depthStateInfo.depthWriteEnable = VK_TRUE;
  depthStateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
  depthStateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
  depthStateInfo.depthBoundsTestEnable = VK_FALSE;

  // Color Blend State =====
  VkPipelineColorBlendAttachmentState blendAttachmentStates [4] {};
  blendAttachmentStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                            VK_COLOR_COMPONENT_G_BIT |
                                            VK_COLOR_COMPONENT_B_BIT |
                                            VK_COLOR_COMPONENT_A_BIT;
  blendAttachmentStates[0].blendEnable = VK_FALSE;
  blendAttachmentStates[1] = blendAttachmentStates[0];
  blendAttachmentStates[2] = blendAttachmentStates[0];
  blendAttachmentStates[3] = blendAttachmentStates[0];

  VkPipelineColorBlendStateCreateInfo blendStateInfo {};
  blendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  blendStateInfo.logicOpEnable = VK_FALSE;
  blendStateInfo.attachmentCount = 1;
  blendStateInfo.pAttachments = &blendAttachmentStates[0];

  // Dynamic States =====
  // States included here are capable of being set by dynamic state setting functions
  // When binding the pipeline, if these states are not set via one of the state setting functions
  //    the state's settings bound from the previous bound pipeline shall be used
  VkPipelineDynamicStateCreateInfo dynamicStateInfo {};
  dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicStateInfo.dynamicStateCount = 0;
  dynamicStateInfo.pDynamicStates = nullptr;

  // Creation =====
  VkGraphicsPipelineCreateInfo createInfo { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
  createInfo.pViewportState      = &viewportStateInfo;
  createInfo.pVertexInputState   = &vertexInputStateInfo;
  createInfo.pInputAssemblyState = &inputAssemblyStateInfo;
  createInfo.pRasterizationState = &rasterStateInfo;
  createInfo.pMultisampleState   = &multisampleStateInfo;
  createInfo.pDepthStencilState  = &depthStateInfo;
  createInfo.pColorBlendState    = &blendStateInfo;
  createInfo.pDynamicState       = &dynamicStateInfo;
  createInfo.stageCount = stages.size();
  createInfo.pStages    = stages.data();

  createInfo.layout     = _material->ivkPipelineLayout;
  createInfo.renderPass = context.forward.renderpass;
  createInfo.subpass = _material->settings->subpassIndex;

  IVK_ASSERT(vkCreateGraphicsPipelines(context.device,
                                       nullptr,
                                       1,
                                       &createInfo,
                                       context.alloc,
                                       &_material->ivkPipeline),
             "Failed to create the pipeline");

  return true;
}
