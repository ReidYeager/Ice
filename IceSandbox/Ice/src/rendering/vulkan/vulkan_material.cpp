
#include "defines.h"
#include "logger.h"

#include "rendering/vulkan/renderer_vulkan.h"

#include "core/camera.h"
#include "math/vector.h"
#include "platform/file_system.h"

#include <vector>
#include <string>

u32 IvkRenderer::CreateMaterial(const std::vector<IceShaderInfo>& _shaders)
{
  IceLogInfo("===== Creating material %u", materials.size());

  IvkMaterial material;

  // Load shader files =====
  for (const auto& shader : _shaders)
  {
    std::string name = shader.directory;
    std::string tmpName = name;
    if (shader.stages & Ice_Shader_Vertex)
    {
      tmpName.append(".vert");
      CreateShaderModule(&material.vertexModule.module, tmpName.c_str());
      material.vertexModule.info = shader;
      tmpName = name;
    }
    if (shader.stages & Ice_Shader_Fragment)
    {
      tmpName.append(".frag");
      CreateShaderModule(&material.fragmentModule.module, tmpName.c_str());
      material.fragmentModule.info = shader;
      tmpName = name;
    }
  }

  ICE_ATTEMPT(CreateDescriptorSet(material));
  ICE_ATTEMPT(CreatePipelinelayout(material));
  ICE_ATTEMPT(CreatePipeline(material));

  if (texture.sampler == VK_NULL_HANDLE)
  {
    ICE_ATTEMPT(CreateTexture(&texture, "TestImage.png"));
  }

  // TODO : Create a function that handles this automatically
  VkDescriptorImageInfo imageInfo {};
  imageInfo.imageLayout = texture.layout;
  imageInfo.imageView = texture.view;
  imageInfo.sampler = texture.sampler;

  VkWriteDescriptorSet write { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
  write.dstSet           = material.descriptorSet;
  write.dstBinding       = 0;
  write.dstArrayElement  = 0;
  write.descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  write.descriptorCount  = 1;
  write.pBufferInfo      = nullptr;
  write.pImageInfo       = &imageInfo;
  write.pTexelBufferView = nullptr;

  vkUpdateDescriptorSets(context.device, 1, &write, 0, nullptr);

  IceLogInfo("===== Finished creating material %u", materials.size());

  materials.push_back(material);

  scene.resize(materials.size()); // Bad.
  return materials.size() - 1;
}

b8 IvkRenderer::CreateDescriptorSet(IvkMaterial& material)
{
  // Create layout =====
  {
    // TODO : Pull descriptors from the shaders automatically
    VkDescriptorSetLayoutBinding textureBinding;
    textureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureBinding.descriptorCount = 1;
    textureBinding.binding = 0;
    textureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    textureBinding.pImmutableSamplers = nullptr;

    const u32 bindingCount = 1;
    VkDescriptorSetLayoutBinding bindings[bindingCount] = { textureBinding };

    VkDescriptorSetLayoutCreateInfo createInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    createInfo.flags = 0;
    createInfo.pNext = nullptr;
    createInfo.bindingCount = bindingCount;
    createInfo.pBindings    = bindings;

    IVK_ASSERT(vkCreateDescriptorSetLayout(context.device,
                                           &createInfo,
                                           context.alloc,
                                           &material.descriptorSetLayout),
               "Failed to create descriptor set layout");
  }

  // Create set =====
  {
    VkDescriptorSetAllocateInfo allocInfo { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    allocInfo.descriptorPool = context.descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &material.descriptorSetLayout;

    IVK_ASSERT(vkAllocateDescriptorSets(context.device, &allocInfo, &material.descriptorSet),
               "Failed to allocate the descriptor set");
  }

  return true;
}

b8 IvkRenderer::CreatePipelinelayout(IvkMaterial& material)
{
  const u32 pushCount = 0;
  //VkPushConstantRange pushRanges[pushCount] = {};
  VkPushConstantRange* pushRanges = nullptr;

  const u32 layoutCount = 2;
  VkDescriptorSetLayout layouts[layoutCount] = {
    context.globalDescriptorSetLayout,
    material.descriptorSetLayout
  };
  //VkDescriptorSetLayout* layouts = nullptr;

  VkPipelineLayoutCreateInfo createInfo { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
  createInfo.flags = 0;
  createInfo.pNext = 0;
  createInfo.pushConstantRangeCount = pushCount;
  createInfo.pPushConstantRanges    = pushRanges;
  createInfo.setLayoutCount = layoutCount;
  createInfo.pSetLayouts    = layouts;

  IVK_ASSERT(vkCreatePipelineLayout(context.device,
                                    &createInfo,
                                    context.alloc,
                                    &material.pipelineLayout),
             "Failed to create the render pipeline");

  return true;
}

b8 IvkRenderer::CreatePipeline(IvkMaterial& material)
{
  // Shader Stages State =====
  // Insert shader modules
  const u32 shaderCount = 2;
  VkPipelineShaderStageCreateInfo shaderStageInfos[shaderCount]{};

  shaderStageInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStageInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  shaderStageInfos[0].module = material.vertexModule.module;
  shaderStageInfos[0].pName = "main";

  shaderStageInfos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStageInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  shaderStageInfos[1].module = material.fragmentModule.module;
  shaderStageInfos[1].pName = "main";

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
  shadowViewport.width = 1024.0f;
  shadowViewport.height = 1024.0f;
  shadowViewport.minDepth = 0;
  shadowViewport.maxDepth = 1;

  VkRect2D shadowScissor{};
  shadowScissor.extent = { 1024, 1024 };
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
  VkPipelineColorBlendAttachmentState blendAttachmentState {};
  blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                        VK_COLOR_COMPONENT_G_BIT |
                                        VK_COLOR_COMPONENT_B_BIT |
                                        VK_COLOR_COMPONENT_A_BIT;
  blendAttachmentState.blendEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo blendStateInfo {};
  blendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  blendStateInfo.logicOpEnable = VK_FALSE;
  blendStateInfo.attachmentCount = 1;
  blendStateInfo.pAttachments = &blendAttachmentState;

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
  createInfo.stageCount = shaderCount;
  createInfo.pStages    = shaderStageInfos;
  createInfo.renderPass = context.renderpass;
  createInfo.layout     = material.pipelineLayout;

  IVK_ASSERT(vkCreateGraphicsPipelines(context.device,
                                       nullptr,
                                       1,
                                       &createInfo,
                                       context.alloc,
                                       &material.pipeline),
             "Failed to create the graphics pipeline");

  createInfo.stageCount = 1;
  createInfo.pStages = shaderStageInfos;
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
  std::string directory = ICE_RESOURCE_SHADER_DIR;
  directory.append(_shader);
  directory.append(".spv");

  std::vector<char> source = FileSystem::LoadFile(directory.c_str());

  VkShaderModuleCreateInfo createInfo { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
  createInfo.codeSize = source.size();
  createInfo.pCode = reinterpret_cast<u32*>(source.data());

  IVK_ASSERT(vkCreateShaderModule(context.device, &createInfo, context.alloc, _module),
             "failed to create shader module for %s", directory.c_str());

  return true;
}
