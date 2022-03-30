
#include "defines.h"

#include "rendering/vulkan/vulkan.h"
#include "platform/platform.h"
#include "tools/lexer.h"

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

  CreateShaderModule(&newShader);
  LoadShaderDescriptors(&newShader);

  return newShader;
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

u32 TokenIsValidFor(const Ice::LexerToken& _token, const char* const* _stringArray, u32 _count)
{
  u32 index;
  for (index = 0; index < _count; index++)
  {
    if (_token.string.compare(_stringArray[index]) == 0)
      break;
  }

  return index;
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
        u32 typeIndex = TokenIsValidFor(token,
                                        Ice::ShaderBufferComponentStrings,
                                        (u32)Ice::Shader_Buffer_Count);
        if (typeIndex == (u32)Ice::Shader_Buffer_Count)
        {
          IceLogWarning("Ivalid buffer component '%s' in descriptor file '%s'",
                        token.string.c_str(),
                        directory.c_str());
          continue;
        }

        // Include the buffer component
        _shader->bufferSize += 16; // Each buffer component represents 16 bytes
      }
    }

    // Get descriptor information =====
    if (lexer.ExpectString("bindings") && lexer.ExpectString("{"))
    {
      while (!lexer.ExpectString("}"))
      {
        token = lexer.NextToken();

        // Check if the token is a valid descriptor
        u32 typeIndex = TokenIsValidFor(token,
                                        Ice::ShaderInputTypeStrings,
                                        (u32)Ice::Shader_Input_Count);
        if (typeIndex == (u32)Ice::Shader_Input_Count)
        {
          IceLogWarning("Ivalid descriptor '%s' in descriptor file '%s'",
                        token.string.c_str(),
                        directory.c_str());
          continue;
        }

        if (!lexer.ExpectType(Ice::Token_Int, &token))
        {
          IceLogError("Binding line %u is missing an index. Using line as index.\n> In '%s'",
                      _shader->input.size(),
                      _shader->fileDirectory.c_str());

          newInput.inputIndex = _shader->input.size();
        }
        else
        {
          newInput.inputIndex = lexer.GetUIntFromToken(&token);
        }

        // Include the descriptor
        newInput.type = (ShaderInputTypes)typeIndex;
        _shader->input.push_back(newInput);
      }
    }
  }

}

Ice::Material Ice::RendererVulkan::CreateMaterial(Ice::MaterialSettings _settings)
{
  Ice::Material newMaterial {};

  // TODO : ~!!~ Get shaders' descriptors
  // Assign default descriptor values (new buffer, default textures)

  CreatePipelineLayout(_settings, &newMaterial.ivkPipelineLayout);
  CreatePipeline(_settings, &newMaterial);

  return newMaterial;
}

void Ice::RendererVulkan::DestroyMaterial(Ice::Material& _material)
{
  vkDeviceWaitIdle(context.device);

  vkDestroyPipelineLayout(context.device, _material.ivkPipelineLayout, context.alloc);
  vkDestroyPipeline(context.device, _material.ivkPipeline, context.alloc);
}

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

b8 Ice::RendererVulkan::CreatePipeline(Ice::MaterialSettings _settings, Ice::Material* _material)
{
  // Shader Stages State =====
  VkPipelineShaderStageCreateInfo shaderStage {};
  shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
  shaderStage.pName = "main";

  std::vector<VkPipelineShaderStageCreateInfo> stages(_settings.shaders.size());
  for (u32 i = 0; i < _settings.shaders.size(); i++)
  {
    shaderStage.module = _settings.shaders[i].ivkShaderModule;
    switch (_settings.shaders[i].type)
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
  createInfo.stageCount = stages.size();
  createInfo.pStages    = stages.data();

  createInfo.layout     = _material->ivkPipelineLayout;
  createInfo.renderPass = context.forward.renderpass;
  createInfo.subpass = _settings.subpassIndex;

  IVK_ASSERT(vkCreateGraphicsPipelines(context.device,
                                       nullptr,
                                       1,
                                       &createInfo,
                                       context.alloc,
                                       &_material->ivkPipeline),
             "Failed to create the pipeline");

  return true;
}
