
#include "defines.h"
#include "logger.h"

#include "rendering/vulkan/vulkan_renderer.h"


IvkAttachmentDescRef IvkRenderer::CreateAttachment(IvkAttachmentSettings _settings)
{
  IvkAttachmentDescRef descRef {};

  descRef.description.format = _settings.imageFormat;
  descRef.description.samples = VK_SAMPLE_COUNT_1_BIT;
  descRef.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  descRef.description.finalLayout = _settings.finalLayout;
  descRef.description.loadOp = _settings.loadOperation;
  descRef.description.storeOp = _settings.storeOperation;
  descRef.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  descRef.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  descRef.reference.attachment = _settings.index;
  descRef.reference.layout = _settings.referenceLayout;

  return descRef;
}

b8 IvkRenderer::CreateRenderpass()
{
  IvkAttachmentSettings attachSettings {};

  // Color =====
  attachSettings.index = 0;
  attachSettings.imageFormat = context.swapchainFormat;
  attachSettings.loadOperation   = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachSettings.storeOperation  = VK_ATTACHMENT_STORE_OP_STORE;
  attachSettings.finalLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  attachSettings.referenceLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  IvkAttachmentDescRef color = CreateAttachment(attachSettings);

  // Depth =====
  attachSettings.index = 1;
  attachSettings.imageFormat = context.depthImage.format;
  attachSettings.loadOperation   = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachSettings.storeOperation  = VK_ATTACHMENT_STORE_OP_STORE;
  attachSettings.finalLayout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  attachSettings.referenceLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  IvkAttachmentDescRef depth = CreateAttachment(attachSettings);

  // Subpass =====
  VkSubpassDescription subpassDescription {};
  subpassDescription.colorAttachmentCount = 1;
  subpassDescription.pColorAttachments = &color.reference;
  subpassDescription.pDepthStencilAttachment = &depth.reference;
  subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

  VkSubpassDependency subpassDependency {};
  subpassDependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
  subpassDependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.srcAccessMask = 0;
  subpassDependency.dstSubpass    = 0;
  subpassDependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  // Creation =====
  const u32 attachemntCount = 2;
  VkAttachmentDescription attachments[attachemntCount] = { color.description, depth.description };
  const u32 subpassCount = 1;
  VkSubpassDescription subpasses[subpassCount] = { subpassDescription };
  const u32 dependencyCount = 1;
  VkSubpassDependency dependencies[dependencyCount] = { subpassDependency };

  VkRenderPassCreateInfo createInfo { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
  createInfo.flags = 0;
  createInfo.attachmentCount = attachemntCount;
  createInfo.pAttachments    = attachments;
  createInfo.subpassCount = subpassCount;
  createInfo.pSubpasses   = subpasses;
  createInfo.dependencyCount = dependencyCount;
  createInfo.pDependencies   = dependencies;

  IVK_ASSERT(vkCreateRenderPass(context.device, &createInfo, context.alloc, &context.renderpass),
             "Failed to create renderpass");

  return true;
}

b8 IvkRenderer::CreateShadowRenderpass()
{
  // Attachments =====
  IvkAttachmentSettings attachSettings {};
  attachSettings.index = 0;
  attachSettings.imageFormat = shadow.image.format;
  attachSettings.loadOperation   = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachSettings.storeOperation  = VK_ATTACHMENT_STORE_OP_STORE;
  attachSettings.finalLayout     = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR;
  attachSettings.referenceLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR;

  IvkAttachmentDescRef shadowDepth = CreateAttachment(attachSettings);

  // Subpass =====
  VkSubpassDescription subpassDescription {};
  subpassDescription.colorAttachmentCount = 0;
  subpassDescription.pColorAttachments = nullptr;
  subpassDescription.pDepthStencilAttachment = &shadowDepth.reference;
  subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

  VkSubpassDependency subpassDependency {};
  subpassDependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
  subpassDependency.srcStageMask  = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  subpassDependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  subpassDependency.dstSubpass    = 0;
  subpassDependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  // Creation =====
  const u32 attachemntCount = 1;
  VkAttachmentDescription attachments[attachemntCount] = { shadowDepth.description };
  const u32 subpassCount = 1;
  VkSubpassDescription subpasses[subpassCount] = { subpassDescription };
  const u32 dependencyCount = 1;
  VkSubpassDependency dependencies[dependencyCount] = { subpassDependency };

  VkRenderPassCreateInfo createInfo { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
  createInfo.flags = 0;
  createInfo.attachmentCount = attachemntCount;
  createInfo.pAttachments    = attachments;
  createInfo.subpassCount = subpassCount;
  createInfo.pSubpasses   = subpasses;
  createInfo.dependencyCount = dependencyCount;
  createInfo.pDependencies   = dependencies;

  IVK_ASSERT(vkCreateRenderPass(context.device, &createInfo, context.alloc, &shadow.renderpass),
              "Failed to create shadow renderpass");

  return true;
}

b8 IvkRenderer::CreateDepthImage()
{
  // Find depth format =====
  VkFormat format = VK_FORMAT_D32_SFLOAT;
  VkFormatProperties formatProperties;
  const u32 fCount = 3;
  VkFormat desiredFormats[fCount] = { VK_FORMAT_D32_SFLOAT,
                                      VK_FORMAT_D32_SFLOAT_S8_UINT,
                                      VK_FORMAT_D24_UNORM_S8_UINT };
  for (u32 i = 0; i < fCount; i++)
  {
    vkGetPhysicalDeviceFormatProperties(context.gpu.device, desiredFormats[i], &formatProperties);

    if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
      format = desiredFormats[i];
      break;
    }
  }

  context.depthImage.format = format;

  // Create image =====
  ICE_ATTEMPT(CreateImage(&context.depthImage,
                          context.swapchainExtent,
                          format,
                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT));
  context.depthImage.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
  // Create image view =====
  ICE_ATTEMPT(CreateImageView(&context.depthImage.view,
                              context.depthImage.image,
                              context.depthImage.format,
                              VK_IMAGE_ASPECT_DEPTH_BIT));

  return true;
}

b8 IvkRenderer::CreateFrameBuffer(VkFramebuffer* _framebuffer,
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

b8 IvkRenderer::CreateShadowFrameBuffers()
{
  ICE_ATTEMPT(CreateFrameBuffer(&shadow.framebuffer,
                                shadow.renderpass,
                                { shadowResolution, shadowResolution },
                                { shadow.image.view }));

  return true;
}

