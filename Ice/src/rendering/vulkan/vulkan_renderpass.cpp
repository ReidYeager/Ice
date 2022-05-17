
#include "defines.h"

#include "rendering/vulkan/vulkan.h"

b8 Ice::RendererVulkan::CreateDepthImages()
{
  const u32 count = context.swapchainImages.size();
  context.depthImages.resize(count);

  for (u32 i = 0; i < count; i++)
  {
    CreateImage(&context.depthImages[i],
                context.swapchainExtent,
                VK_FORMAT_D24_UNORM_S8_UINT,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    CreateImageView(&context.depthImages[i].view,
                    context.depthImages[i].image,
                    context.depthImages[i].format,
                    VK_IMAGE_ASPECT_DEPTH_BIT);
  }

  return true;
}

// =========================
// Renderpasses
// =========================

b8 CreateFrameBuffer(Ice::VulkanContext& context,
                     VkFramebuffer* _framebuffer,
                     VkRenderPass& _renderpass,
                     VkExtent2D _extents,
                     std::vector<VkImageView> _views)
{
  VkFramebufferCreateInfo createInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
  createInfo.flags = 0;
  createInfo.layers = 1;
  createInfo.renderPass = _renderpass;
  createInfo.width = _extents.width;
  createInfo.height = _extents.height;

  createInfo.attachmentCount = _views.size();
  createInfo.pAttachments = _views.data();

  IVK_ASSERT(vkCreateFramebuffer(context.device,
                                 &createInfo,
                                 context.alloc,
                                 _framebuffer),
                                 "Failed to create frame buffer");

  return true;
}

b8 Ice::RendererVulkan::CreateDeferredComponents()
{
  return true;
}

b8 Ice::RendererVulkan::CreateForwardComponents()
{
  // ==========
  // Renderpass
  // ==========

  // Descriptions =====

  const u32 attachmentCount = 2;
  VkAttachmentDescription attachments[attachmentCount];
  // Swapchain
  attachments[0].flags = 0;
  attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[0].format = context.swapchainFormat;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[0].finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  attachments[0].loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  // Depth
  attachments[1].flags = 0;
  attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[1].format = context.depthImages[0].format;
  attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  VkAttachmentReference references[attachmentCount];
  // Swapchain
  references[0].attachment = 0;
  references[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  // Depth
  references[1].attachment = 1;
  references[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  // Subpass =====

  VkSubpassDescription subpass;
  subpass.flags = 0;
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &references[0];
  subpass.pDepthStencilAttachment = &references[1];
  subpass.inputAttachmentCount = 0;
  subpass.pInputAttachments = nullptr;
  subpass.preserveAttachmentCount = 0;
  subpass.pPreserveAttachments = nullptr;
  subpass.pResolveAttachments = nullptr;

  // Dependencies =====

  //VkSubpassDependency dependencies[1];

  //dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
  //dependencies[0].dstSubpass = 0;
  //dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  //dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  //dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  //dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  //dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  //dependencies[1].srcSubpass = 0;
  //dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
  //dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  //dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  //dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  //dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  //dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  // Creation =====
  VkRenderPassCreateInfo rpCreateInfo { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
  rpCreateInfo.flags = 0;
  rpCreateInfo.attachmentCount = attachmentCount;
  rpCreateInfo.pAttachments    = attachments;
  rpCreateInfo.subpassCount = 1;
  rpCreateInfo.pSubpasses   = &subpass;
  rpCreateInfo.dependencyCount = 0;
  //rpCreateInfo.pDependencies   = dependencies;

  IVK_ASSERT(vkCreateRenderPass(context.device,
                                &rpCreateInfo,
                                context.alloc,
                                &context.forward.renderpass),
             "Failed to create forward renderpass");

  // ==========
  // Framebuffers
  // ==========
  u32 count = context.swapchainImages.size();
  context.forward.framebuffers.resize(count);

  for (u32 i = 0; i < count; i++)
  {
    ICE_ATTEMPT(CreateFrameBuffer(context,
                                  &context.forward.framebuffers[i],
                                  context.forward.renderpass,
                                  context.swapchainExtent,
                                  {context.swapchainImageViews[i],
                                  context.depthImages[i].view }));
  }

  return true;
}
