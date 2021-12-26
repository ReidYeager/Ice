
#include "defines.h"
#include "logger.h"

#include "rendering/vulkan/vulkan_renderer.h"


IvkAttachmentDescRef IvkRenderer::CreateAttachment(IvkAttachmentSettings _settings, u32 _index)
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

  descRef.reference.attachment = _index;
  descRef.reference.layout = _settings.referenceLayout;

  return descRef;
}

b8 IvkRenderer::CreateRenderpass()
{
  // Inputs =====
  std::vector<IvkAttachmentSettings> _attachSettings(2);
  // Color
  _attachSettings[0].imageFormat = context.swapchainFormat;
  _attachSettings[0].finalLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  _attachSettings[0].referenceLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  // Depth
  _attachSettings[1].imageFormat = context.depthImage.format;
  _attachSettings[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  _attachSettings[1].referenceLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  std::vector<IvkSubpassSettings> _subpass(1);
  _subpass[0].colorIndices.push_back(0);
  _subpass[0].depthIndex = 1;

  std::vector<IvkSubpassDependancies> _deps(1);
  _deps[0].srcIndex = VK_SUBPASS_EXTERNAL;
  _deps[0].dstIndex = 0;

  // Attachments =====
  std::vector<IvkAttachmentDescRef> attachmentDescRefs(_attachSettings.size());
  for (u32 i = 0; i < attachmentDescRefs.size(); i++)
  {
    attachmentDescRefs[i] = CreateAttachment(_attachSettings[i], i);
  }

  // Subpass =====
  std::vector<VkAttachmentReference> colorRefs(_subpass[0].colorIndices.size());
  for (u32 i = 0; i < _subpass[0].colorIndices.size(); i++)
  {
    colorRefs[i] = attachmentDescRefs[_subpass[0].colorIndices[i]].reference;
  }

  VkSubpassDescription subpassDescription {};
  subpassDescription.colorAttachmentCount = colorRefs.size();
  subpassDescription.pColorAttachments = colorRefs.data();
  subpassDescription.pDepthStencilAttachment = &attachmentDescRefs[_subpass[0].depthIndex].reference;
  subpassDescription.pipelineBindPoint = _subpass[0].bindPoint;

  VkSubpassDependency subpassDependency {};
  subpassDependency.srcSubpass    = _deps[0].srcIndex;
  subpassDependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.dstSubpass    = _deps[0].dstIndex;
  subpassDependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  // Creation =====
  const u32 attachemntCount = 2;
  VkAttachmentDescription attachments[attachemntCount] = { attachmentDescRefs[0].description,
                                                           attachmentDescRefs[1].description};
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
  attachSettings.imageFormat = shadow.image.format;
  attachSettings.finalLayout     = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR;
  attachSettings.referenceLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR;

  IvkAttachmentDescRef shadowDepth = CreateAttachment(attachSettings, 0);

  // Subpass =====
  VkSubpassDescription subpassDescription {};
  subpassDescription.colorAttachmentCount = 0;
  subpassDescription.pColorAttachments = nullptr;
  subpassDescription.pDepthStencilAttachment = &shadowDepth.reference;
  subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

  VkSubpassDependency subpassDependency {};
  subpassDependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
  subpassDependency.srcStageMask  = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
  subpassDependency.dstSubpass    = 0;
  subpassDependency.dstStageMask  = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;

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

