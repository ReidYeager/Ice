
#include "defines.h"

#include "rendering/vulkan/vulkan_renderer.h"

b8 IvkRenderer::CreateShadowRenderpass()
{
  // Attachments =====
  VkAttachmentDescription attachSettings {};
  attachSettings.format = VK_FORMAT_D16_UNORM;
  attachSettings.flags = 0;
  attachSettings.samples = VK_SAMPLE_COUNT_1_BIT;
  attachSettings.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachSettings.finalLayout   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
  attachSettings.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachSettings.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachSettings.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachSettings.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  VkAttachmentReference reference {};
  reference.attachment = 0;
  reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  // Subpass =====
  VkSubpassDescription subpass {};
  subpass.flags = 0;
  subpass.colorAttachmentCount = 0;
  subpass.pDepthStencilAttachment = &reference;
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

  // Dependencies =====
  VkSubpassDependency dependencies[2];
  dependencies[0].srcSubpass  = VK_SUBPASS_EXTERNAL;
  dependencies[0].dstSubpass = 0;
  dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
  dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  dependencies[1].srcSubpass = 0;
  dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  // Creation =====
  VkRenderPassCreateInfo createInfo { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
  createInfo.flags = 0;
  createInfo.attachmentCount = 1;
  createInfo.pAttachments    = &attachSettings;
  createInfo.subpassCount = 1;
  createInfo.pSubpasses   = &subpass;
  createInfo.dependencyCount = 2;
  createInfo.pDependencies   = dependencies;

  IVK_ASSERT(vkCreateRenderPass(context.device, &createInfo, context.alloc, &shadow.renderpass),
             "Failed to create shadow renderpass");

  return true;
}

b8 IvkRenderer::CreateShadowFrameBuffer()
{
  // Create depth map =====

  ICE_ATTEMPT_BOOL(CreateImage(&shadow.image,
                          { shadowResolution, shadowResolution },
                          VK_FORMAT_D16_UNORM,
                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT|VK_IMAGE_USAGE_SAMPLED_BIT));
  shadow.image.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

  ICE_ATTEMPT_BOOL(CreateImageView(&shadow.image.view,
                              shadow.image.image,
                              shadow.image.format,
                              VK_IMAGE_ASPECT_DEPTH_BIT));
  ICE_ATTEMPT_BOOL(CreateImageSampler(&shadow.image));

  // Create framebuffer =====

  ICE_ATTEMPT_BOOL(CreateFrameBuffer(&shadow.framebuffer,
                                shadow.renderpass,
                                { shadowResolution, shadowResolution },
                                { shadow.image.view }));

  return true;
}

