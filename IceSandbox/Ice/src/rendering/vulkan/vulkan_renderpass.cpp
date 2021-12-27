
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

b8 IvkRenderer::CreateRenderpass(VkRenderPass* _renderpass,
                                 std::vector<IvkAttachmentSettings> _attachSettings,
                                 std::vector<IvkSubpassSettings> _subpasses,
                                 std::vector<IvkSubpassDependencies> _dependencies)
{
  // Attachments =====
  std::vector<IvkAttachmentDescRef> descRefs(_attachSettings.size());
  std::vector<VkAttachmentDescription> attachments(_attachSettings.size());

  for (u32 i = 0; i < descRefs.size(); i++)
  {
    descRefs[i] = CreateAttachment(_attachSettings[i], i);
    attachments[i] = descRefs[i].description;
  }

  // Subpasses =====
  std::vector<std::vector<VkAttachmentReference>> allColorReferences(_subpasses.size());
  std::vector<VkSubpassDescription> subpasses(_subpasses.size());

  for (u32 i = 0; i < _subpasses.size(); i++)
  {
    std::vector<VkAttachmentReference>& colorRefs = allColorReferences[i];
    colorRefs.resize(_subpasses[i].colorIndices.size());

    for (u32 j = 0; j < _subpasses[i].colorIndices.size(); j++)
    {
      colorRefs[j] = descRefs[_subpasses[i].colorIndices[j]].reference;
    }

    VkSubpassDescription& subpassDescription = subpasses[i];
    subpassDescription.pipelineBindPoint = _subpasses[i].bindPoint;

    subpassDescription.colorAttachmentCount = colorRefs.size();
    subpassDescription.pColorAttachments = colorRefs.size() == 0 ? nullptr : colorRefs.data();

    if (_subpasses[i].depthIndex == -1)
      subpassDescription.pDepthStencilAttachment = nullptr;
    else
      subpassDescription.pDepthStencilAttachment = &descRefs[_subpasses[i].depthIndex].reference;
  }

  // Dependencies =====
  std::vector<VkSubpassDependency> dependencies(_dependencies.size());

  for (u32 i = 0; i < _dependencies.size(); i++)
  {
    dependencies[i].srcSubpass = _dependencies[i].srcIndex;
    dependencies[i].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[i].dstSubpass = _dependencies[i].dstIndex;
    dependencies[i].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  }

  // Creation =====
  VkRenderPassCreateInfo createInfo { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
  createInfo.flags = 0;
  createInfo.attachmentCount = attachments.size();
  createInfo.pAttachments    = attachments.data();
  createInfo.subpassCount = subpasses.size();
  createInfo.pSubpasses   = subpasses.data();
  createInfo.dependencyCount = dependencies.size();
  createInfo.pDependencies   = dependencies.data();

  IVK_ASSERT(vkCreateRenderPass(context.device, &createInfo, context.alloc, _renderpass),
             "Failed to create renderpass -- %u attachments, %u subpasses, %u dependencies",
             (u32)attachments.size(), (u32)subpasses.size(), (u32)dependencies.size());

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

