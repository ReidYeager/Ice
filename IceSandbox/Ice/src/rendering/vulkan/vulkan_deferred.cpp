
#include "defines.h"

#include "rendering/vulkan/vulkan_renderer.h"

b8 IvkRenderer::CreateDeferredRenderpass()
{
  // Defining formats here to simplify future uses
  context.geoBuffers.push_back({});
  IvkGeoBuffer& gb = context.geoBuffers[0];
  gb.position.format = gb.normal.format = VK_FORMAT_R32G32B32A32_SFLOAT; // 32-bit [signed float]
  gb.albedo.format = gb.maps.format = VK_FORMAT_R8G8B8A8_UNORM, // 8-bit [0 to 1]
  gb.depth.format = GetDepthFormat();

  // Attachments =====
  const u32 attachmentCount = 6;
  VkAttachmentDescription attachments[attachmentCount];

  // Swapchain
  attachments[0].flags = 0;
  attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[0].format = context.swapchainFormat;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  // Position
  attachments[1].flags = 0;
  attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[1].format = gb.position.format;
  attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[1].finalLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  attachments[1].loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  // Normal
  attachments[2].flags = 0;
  attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[2].format = gb.normal.format;
  attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[2].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  // Albedo
  attachments[3].flags = 0;
  attachments[3].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[3].format = gb.albedo.format;
  attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[3].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  // Maps
  attachments[4].flags = 0;
  attachments[4].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[4].format = gb.maps.format;
  attachments[4].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[4].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  attachments[4].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[4].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[4].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[4].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  // Depth
  attachments[5].flags = 0;
  attachments[5].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[5].format = gb.depth.format;
  attachments[5].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[5].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  attachments[5].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[5].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[5].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[5].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  // Subpasses =====
  const u32 subpassCount = 2;
  VkSubpassDescription subpasses[subpassCount];

  // Geometry subpass =====
  VkAttachmentReference geoReferences[5];
  // Position
  geoReferences[0].attachment = 1;
  geoReferences[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  // Normal
  geoReferences[1].attachment = 2;
  geoReferences[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  // Albedo
  geoReferences[2].attachment = 3;
  geoReferences[2].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  // Maps
  geoReferences[3].attachment = 4;
  geoReferences[3].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  // Depth
  geoReferences[4].attachment = 5;
  geoReferences[4].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  subpasses[0].flags = 0;
  subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpasses[0].colorAttachmentCount = 4;
  subpasses[0].pColorAttachments = &geoReferences[0];
  subpasses[0].pDepthStencilAttachment = &geoReferences[4];
  subpasses[0].inputAttachmentCount = 0;
  subpasses[0].pInputAttachments = nullptr;
  subpasses[0].preserveAttachmentCount = 0;
  subpasses[0].pPreserveAttachments = nullptr;
  subpasses[0].pResolveAttachments = nullptr;

  // Lighting subpass =====
  VkAttachmentReference lightingReferences[6];
  // Swapchain
  lightingReferences[0].attachment = 0;
  lightingReferences[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  // Position
  lightingReferences[1].attachment = 1;
  lightingReferences[1].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  // Normal
  lightingReferences[2].attachment = 2;
  lightingReferences[2].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  // Albedo
  lightingReferences[3].attachment = 3;
  lightingReferences[3].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  // Maps
  lightingReferences[4].attachment = 4;
  lightingReferences[4].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  // Depth
  lightingReferences[5].attachment = 5;
  lightingReferences[5].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  subpasses[1].flags = 0;
  subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpasses[1].colorAttachmentCount = 1;
  subpasses[1].pColorAttachments = &lightingReferences[0];
  subpasses[1].pDepthStencilAttachment = nullptr;
  subpasses[1].inputAttachmentCount = 5;
  subpasses[1].pInputAttachments = &lightingReferences[1];
  subpasses[1].preserveAttachmentCount = 0;
  subpasses[1].pPreserveAttachments = nullptr;
  subpasses[1].pResolveAttachments = nullptr;

  // Dependencies =====
  const u32 dependencyCount = 3;
  VkSubpassDependency dependencies[dependencyCount];

  // TODO : --> Learn more about stage/access masks
  // External to geometry subpass
  dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[0].dstSubpass = 0;
  dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
  // Geometry subpass to swapchain subpass
  dependencies[1].srcSubpass = 0;
  dependencies[1].dstSubpass = 1;
  dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
  // Swapchain subpass to external
  dependencies[2].srcSubpass = 1;
  dependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[2].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[2].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  // Creation =====
  VkRenderPassCreateInfo createInfo { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
  createInfo.flags = 0;
  createInfo.attachmentCount = attachmentCount;
  createInfo.pAttachments    = attachments;
  createInfo.subpassCount = subpassCount;
  createInfo.pSubpasses   = subpasses;
  createInfo.dependencyCount = dependencyCount;
  createInfo.pDependencies   = dependencies;

  IVK_ASSERT(vkCreateRenderPass(context.device, &createInfo, context.alloc, &context.deferredRenderpass),
             "Failed to create renderpass");

  return true;
}

b8 IvkRenderer::CreateDeferredFramebuffers()
{
  const u32 count = context.swapchainImages.size();
  VkExtent2D extent = context.swapchainExtent;

  context.geoBuffers.resize(count);
  IvkGeoBuffer& formatBuffers = context.geoBuffers[0];

  for (u32 i = 0; i < count; i++)
  {
    IvkGeoBuffer& gb = context.geoBuffers[i];

    // Position =====
    CreateImage(&gb.position,
                extent,
                formatBuffers.position.format,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    CreateImageView(&gb.position.view,
                    gb.position.image,
                    gb.position.format,
                    VK_IMAGE_ASPECT_COLOR_BIT);
    //gb.position.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    gb.position.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Normal =====
    CreateImage(&gb.normal,
                extent,
                formatBuffers.normal.format,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    CreateImageView(&gb.normal.view,
                    gb.normal.image,
                    gb.normal.format,
                    VK_IMAGE_ASPECT_COLOR_BIT);
    //gb.normal.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    gb.normal.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Albedo =====
    CreateImage(&gb.albedo,
                extent,
                formatBuffers.albedo.format,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    CreateImageView(&gb.albedo.view,
                    gb.albedo.image,
                    gb.albedo.format,
                    VK_IMAGE_ASPECT_COLOR_BIT);
    //gb.albedo.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    gb.albedo.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Maps =====
    CreateImage(&gb.maps,
                extent,
                formatBuffers.maps.format,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    CreateImageView(&gb.maps.view,
                    gb.maps.image,
                    gb.maps.format,
                    VK_IMAGE_ASPECT_COLOR_BIT);
    //gb.maps.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    gb.maps.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Depth =====
    CreateImage(&gb.depth,
                extent,
                formatBuffers.depth.format,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    CreateImageView(&gb.depth.view,
                    gb.depth.image,
                    gb.depth.format,
                    VK_IMAGE_ASPECT_DEPTH_BIT);
    //gb.depth.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    gb.depth.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

    // Framebuffer =====
    CreateFrameBuffer(&gb.framebuffer,
                      context.deferredRenderpass,
                      extent,
                      { context.swapchainImageViews[i],
                        gb.position.view,
                        gb.normal.view,
                        gb.albedo.view,
                        gb.maps.view,
                        gb.depth.view });

  }

  return true;
}

