
#include "defines.h"
#include "logger.h"

#include "rendering/vulkan/vulkan_renderer.h"
#include "rendering/render_context.h"

#include "core/camera.h"
#include "math/vector.h"
#include "platform/file_system.h"

#include <vector>
#include <string>

IceHandle IvkRenderer::CreateShader(const std::string _dir, const IceShaderStage _stage)
{
  IvkShader newShader {};

  switch (_stage)
  {
  case Ice_Shader_Vertex:
  {
    newShader.stage = VK_SHADER_STAGE_VERTEX_BIT;
  } break;
  case Ice_Shader_Fragment:
  {
    newShader.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  } break;
  }

  if (!CreateShaderModule(&newShader.module, _dir.c_str()))
    return ICE_NULL_HANDLE;

  IceHandle index = shaders.size();
  shaders.push_back(newShader);

  return index;
}

b8 IvkRenderer::RecreateShader(const IceShader& _shader)
{
  vkDeviceWaitIdle(context.device);

  vkDestroyShaderModule(context.device, shaders[_shader.backendShader].module, context.alloc);
  std::string fullDir = _shader.directory;
  switch (_shader.stage)
  {
  case Ice_Shader_Vertex:
  {
    fullDir.append(".vert");
  } break;
  case Ice_Shader_Fragment:
  {
    fullDir.append(".frag");
  } break;
  }
  return CreateShaderModule(&shaders[_shader.backendShader].module, fullDir.c_str());
}

u32 IvkRenderer::CreateMaterial(const std::vector<IceHandle>& _shaders,
                                std::vector<IceShaderBinding>& _descBindings,
                                u32 _subpassIndex)
{
  IvkMaterial material;

  material.shaderIndices = _shaders;
  material.subpassIndex = _subpassIndex;

  // Set descriptors =====

  std::vector<IvkDescriptor> descriptors;

  for (auto& d : _descBindings)
  {
    IvkDescriptor newdesc{};
    newdesc.binding = d.descriptor.bindingIndex;

    switch (d.descriptor.type)
    {
    case Ice_Descriptor_Type_Buffer:
    {
      newdesc.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    } break;
    case Ice_Descriptor_Type_Sampler2D:
    {
      newdesc.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    } break;
    case Ice_Descriptor_Type_SubpassInput:
    {
      newdesc.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
      newdesc.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    } break;
    }

    descriptors.push_back(newdesc);
  }

  ICE_ATTEMPT(CreateDescriptorSet(descriptors,
                                  &material.descriptorSetLayout,
                                  &material.descriptorSet));

  ICE_ATTEMPT(CreatePipelinelayout(&material.pipelineLayout,
                                   { context.globalDescriptorSetLayout,
                                     material.descriptorSetLayout,
                                     context.objectDescriptorSetLayout },
                                   {}));

  // Pipeline =====
  ICE_ATTEMPT(CreatePipeline(material, _subpassIndex));

  // Complete =====
  materials.push_back(material);

  scene.resize(materials.size()); // Bad.
  return materials.size() - 1;
}

void IvkRenderer::AssignMaterialTextures(IceHandle _material, std::vector<u32> _textureIndices)
{
  std::vector<IvkDescriptorBinding> descriptorBindings;

  for (const auto& t : _textureIndices)
  {
    descriptorBindings.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                   &textures[t].image,
                                   nullptr });
  }

  UpdateDescriptorSet(materials[_material].descriptorSet, descriptorBindings);
}

b8 IvkRenderer::RecreateMaterial(IceHandle _backendMaterial,
                                 const std::vector<IceHandle>& _shaders,
                                 std::vector<IceShaderBinding>& _descBindings)
{
  auto& m = materials[_backendMaterial];
  // Destroy fragile components =====
  {
    vkDestroyPipeline(context.device, m.shadowPipeline, context.alloc);
    vkDestroyPipeline(context.device, m.pipeline, context.alloc);
    vkDestroyPipelineLayout(context.device, m.pipelineLayout, context.alloc);
  }

  // Re-create fragile components =====
  {
    ICE_ATTEMPT(CreatePipelinelayout(&m.pipelineLayout,
                                      { context.globalDescriptorSetLayout,
                                        m.descriptorSetLayout,
                                        context.objectDescriptorSetLayout },
                                      {}));

    ICE_ATTEMPT(CreatePipeline(m, m.subpassIndex));
  }

  return true;
}

b8 IvkRenderer::CreateDescriptorSet(std::vector<IvkDescriptor>& _descriptors,
                                    VkDescriptorSetLayout* _setLayout,
                                    VkDescriptorSet* _set)
{
  const u32 count = _descriptors.size();

  // Create layout =====
  {
    std::vector<VkDescriptorSetLayoutBinding> bindings(count);

    for (u32 i = 0; i < count; i++)
    {
      bindings[i].descriptorCount = 1;
      bindings[i].binding = i;
      bindings[i].pImmutableSamplers = nullptr;
      // Customizable =====
      bindings[i].descriptorType = _descriptors[i].type;
      bindings[i].stageFlags = _descriptors[i].stageFlags;
    }

    VkDescriptorSetLayoutCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.flags = 0;
    createInfo.pNext = nullptr;
    createInfo.bindingCount = count;
    createInfo.pBindings    = bindings.data();

    IVK_ASSERT(vkCreateDescriptorSetLayout(context.device,
                                           &createInfo,
                                           context.alloc,
                                           _setLayout),
               "Failed to create descriptor set layout");
  }

  // Create set =====
  {
    VkDescriptorSetAllocateInfo allocInfo { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    allocInfo.descriptorPool = context.descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = _setLayout;

    IVK_ASSERT(vkAllocateDescriptorSets(context.device, &allocInfo, _set),
               "Failed to allocate the descriptor set");
  }

  return true;
}

b8 IvkRenderer::CreatePipelinelayout(VkPipelineLayout* _pipelineLayout,
                                     std::vector<VkDescriptorSetLayout> _descriptorLayouts,
                                     std::vector<VkPushConstantRange> _pushRanges)
{
  VkPipelineLayoutCreateInfo createInfo { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
  createInfo.flags = 0;
  createInfo.pNext = 0;
  createInfo.pushConstantRangeCount = _pushRanges.size();
  createInfo.pPushConstantRanges    = _pushRanges.data();
  createInfo.setLayoutCount = _descriptorLayouts.size();
  createInfo.pSetLayouts    = _descriptorLayouts.data();

  IVK_ASSERT(vkCreatePipelineLayout(context.device,
                                    &createInfo,
                                    context.alloc,
                                    _pipelineLayout),
             "Failed to create the render pipeline");

  return true;
}

b8 IvkRenderer::CreatePipeline(IvkMaterial& material, u32 _subpass)
{
  // Shader Stages State =====
  // Insert shader modules

  VkPipelineShaderStageCreateInfo vertexOnlyShaderStageInfo {};
  std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfos(material.shaderIndices.size());
  for (u32 i = 0, shaderIndex = 0; i < shaderStageInfos.size(); i++)
  {
    shaderIndex = material.shaderIndices[i];
    shaderStageInfos[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfos[i].stage = shaders[shaderIndex].stage;
    shaderStageInfos[i].module = shaders[shaderIndex].module;
    shaderStageInfos[i].pName = "main";

    if (shaders[shaderIndex].stage == VK_SHADER_STAGE_VERTEX_BIT)
    {
      vertexOnlyShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      vertexOnlyShaderStageInfo.stage = shaders[shaderIndex].stage;
      vertexOnlyShaderStageInfo.module = shaders[shaderIndex].module;
      vertexOnlyShaderStageInfo.pName = "main";
    }
  }

  // Vertex Input State =====
  // Defines how vertex buffers are to be traversed
  const auto vertexInputAttribDesc = iceVertex::GetAttributeDescriptions();
  const auto vertexInputBindingDesc = iceVertex::GetBindingDescription();

  VkPipelineVertexInputStateCreateInfo vertexInputStateInfo {};
  vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputStateInfo.vertexAttributeDescriptionCount = vertexInputAttribDesc.size();
  vertexInputStateInfo.pVertexAttributeDescriptions    = vertexInputAttribDesc.data();
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
  VkViewport viewport;
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

  // Shadow
  VkViewport shadowViewport;
  shadowViewport.x = 0;
  shadowViewport.y = 0;
  shadowViewport.width = (float)shadowResolution;
  shadowViewport.height = (float)shadowResolution;
  shadowViewport.minDepth = 0;
  shadowViewport.maxDepth = 1;

  VkRect2D shadowScissor{};
  shadowScissor.extent = { shadowResolution, shadowResolution };
  shadowScissor.offset = { 0, 0 };

  VkPipelineViewportStateCreateInfo shadowViewportStateInfo{};
  shadowViewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  shadowViewportStateInfo.viewportCount = 1;
  shadowViewportStateInfo.pViewports = &shadowViewport;
  shadowViewportStateInfo.scissorCount = 1;
  shadowViewportStateInfo.pScissors = &shadowScissor;

  // Rasterization State =====
  // Defines how the pipeline will rasterize the image
  VkPipelineRasterizationStateCreateInfo rasterStateInfo {};
  rasterStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
  rasterStateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterStateInfo.cullMode = VK_CULL_MODE_BACK_BIT;

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
  blendStateInfo.attachmentCount = _subpass == 0 ? 4 : 1;
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
  createInfo.stageCount = shaderStageInfos.size();
  createInfo.pStages    = shaderStageInfos.data();
  createInfo.renderPass = context.deferredRenderpass;
  createInfo.layout     = material.pipelineLayout;

  createInfo.subpass = _subpass;

  IVK_ASSERT(vkCreateGraphicsPipelines(context.device,
                                       nullptr,
                                       1,
                                       &createInfo,
                                       context.alloc,
                                       &material.pipeline),
             "Failed to create the graphics pipeline");

  // Shadows =====

  if (vertexOnlyShaderStageInfo.sType == 0)
  {
    return true; // No shadow
  }

  createInfo.subpass = 0;
  rasterStateInfo.cullMode = VK_CULL_MODE_FRONT_BIT; // Fix peter-panning
  createInfo.stageCount = 1;
  createInfo.pStages = &vertexOnlyShaderStageInfo;
  createInfo.renderPass = shadow.renderpass;
  createInfo.pViewportState = &shadowViewportStateInfo;
  IVK_ASSERT(vkCreateGraphicsPipelines(context.device,
                                       nullptr,
                                       1,
                                       &createInfo,
                                       context.alloc,
                                       &material.shadowPipeline),
             "Failed to create the shadow pipeline");

  return true;
}

b8 IvkRenderer::CreateShaderModule(VkShaderModule* _module, const char* _shader)
{
  std::string directory = _shader;
  directory.append(".spv");

  std::vector<char> source = fileSystem.LoadFile(directory.c_str());
  if (source.size() == 0)
    return false;

  VkShaderModuleCreateInfo createInfo { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
  createInfo.codeSize = source.size();
  createInfo.pCode = reinterpret_cast<u32*>(source.data());

  IVK_ASSERT(vkCreateShaderModule(context.device, &createInfo, context.alloc, _module),
             "failed to create shader module for %s", directory.c_str());

  return true;
}
