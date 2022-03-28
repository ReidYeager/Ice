
#include "defines.h"

#include "rendering/vulkan/vulkan.h"
#include "platform/platform.h"

#include <string>
#include <vector>

Ice::Shader Ice::RendererVulkan::CreateShader(const Ice::Shader _shader)
{
  Ice::Shader newShader = {_shader.type};

  newShader.fileDirectory = ICE_RESOURCE_SHADER_DIR;
  newShader.fileDirectory.append(_shader.fileDirectory.c_str());
  switch (_shader.type)
  {
  case Shader_Vertex: newShader.fileDirectory.append(".vert"); break;
  case Shader_Fragment: newShader.fileDirectory.append(".frag"); break;
  case Shader_Compute: newShader.fileDirectory.append(".comp"); break;
  default: IceLogError("Shader type unknown"); return {};
  }

  CreateShaderModule(&(VkShaderModule)newShader.apiData[0], newShader.fileDirectory.c_str());

  return newShader;
}

void Ice::RendererVulkan::DestroyShader(Ice::Shader& _shader)
{
  vkDestroyShaderModule(context.device, (VkShaderModule)_shader.apiData[0], context.alloc);
}

b8 Ice::RendererVulkan::CreateShaderModule(VkShaderModule* _module, const char* _shader)
{
  std::string directory = _shader;
  directory.append(".spv");

  std::vector<char> source = Ice::LoadFile(directory.c_str());
  if (source.size() == 0)
    return false;

  VkShaderModuleCreateInfo createInfo { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
  createInfo.codeSize = source.size();
  createInfo.pCode = reinterpret_cast<u32*>(source.data());

  IVK_ASSERT(vkCreateShaderModule(context.device, &createInfo, context.alloc, _module),
             "failed to create shader module for %s", directory.c_str());

  return true;
}

Ice::Material Ice::RendererVulkan::CreateMaterial(Ice::MaterialSettings _settings)
{
  Ice::Material newMaterial {};

  // Get shaders' descriptors
  // Assign default descriptor values (new buffer, default textures)

  CreatePipelineLayout(_settings, &(VkPipelineLayout)newMaterial.apiData[0]);
  CreatePipeline(_settings, &(VkPipeline)newMaterial.apiData[1]);

  return newMaterial;
}

void Ice::RendererVulkan::DestroyMaterial(Ice::Material& _material)
{
  vkDestroyPipelineLayout(context.device, (VkPipelineLayout)_material.apiData[0], context.alloc);
  vkDestroyPipeline(context.device, (VkPipeline)_material.apiData[1], context.alloc);
}

// TODO : Rewrite with materials
b8 Ice::RendererVulkan::CreatePipelineLayout(const Ice::MaterialSettings& _settings,
                                             VkPipelineLayout* _layout)
{
  VkPipelineLayoutCreateInfo createInfo { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
  createInfo.flags = 0;
  createInfo.pNext = nullptr;
  createInfo.pushConstantRangeCount = 0;
  createInfo.pPushConstantRanges = nullptr;
  createInfo.setLayoutCount = 0;
  createInfo.pSetLayouts = nullptr;

  IVK_ASSERT(vkCreatePipelineLayout(context.device,
                                    &createInfo,
                                    context.alloc,
                                    _layout),
             "Failed to create pipeline layout");

  return true;
}

// TODO : Rewrite with materials
b8 Ice::RendererVulkan::CreatePipeline(const Ice::MaterialSettings& _settings,
                                       VkPipeline* _pipeline)
{
  // Shader Stages State =====
  // Insert shader modules

  std::string vertDirectory = ICE_RESOURCE_SHADER_DIR;
  vertDirectory.append("_light_blank.vert");
  VkShaderModule vertModule;
  CreateShaderModule(&vertModule, vertDirectory.c_str());
  VkPipelineShaderStageCreateInfo vertShaderStage {};
  vertShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStage.module = vertModule;
  vertShaderStage.pName = "main";

  std::string fragDirectory = ICE_RESOURCE_SHADER_DIR;
  fragDirectory.append("blank_forward.frag");
  VkShaderModule fragModule;
  CreateShaderModule(&fragModule, fragDirectory.c_str());
  VkPipelineShaderStageCreateInfo fragShaderStage{};
  fragShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStage.module = fragModule;
  fragShaderStage.pName = "main";

  VkPipelineShaderStageCreateInfo stages[2] = { vertShaderStage, fragShaderStage };

  // Vertex Input State =====
  // Defines how vertex buffers are to be traversed
  const auto vertexInputAttribDesc = Ice::IvkVertex::GetAttributeDescriptions();
  const auto vertexInputBindingDesc = Ice::IvkVertex::GetBindingDescription();

  VkPipelineVertexInputStateCreateInfo vertexInputStateInfo {};
  vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputStateInfo.vertexAttributeDescriptionCount = 0;
  //vertexInputStateInfo.vertexAttributeDescriptionCount = vertexInputAttribDesc.size();
  //vertexInputStateInfo.pVertexAttributeDescriptions    = vertexInputAttribDesc.data();
  vertexInputStateInfo.vertexBindingDescriptionCount = 0;
  //vertexInputStateInfo.vertexBindingDescriptionCount = 1;
  //vertexInputStateInfo.pVertexBindingDescriptions    = &vertexInputBindingDesc;

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
  createInfo.stageCount = 2;
  createInfo.pStages    = stages;

  // TODO : Rewrite with materials
  createInfo.layout     = context.pipelineLayout;
  createInfo.renderPass = context.forward.renderpass;
  createInfo.subpass = 0;

  IVK_ASSERT(vkCreateGraphicsPipelines(context.device,
                                       nullptr,
                                       1,
                                       &createInfo,
                                       context.alloc,
                                       _pipeline),
             "Failed to create the pipeline");

  vkDestroyShaderModule(context.device, vertModule, context.alloc);
  vkDestroyShaderModule(context.device, fragModule, context.alloc);

  return true;
}
