
#include "defines.h"
#include "logger.h"

#include "rendering/vulkan/vulkan.h"
#include "rendering/vulkan/vulkan_context.h"

#include <vector>

b8 Ice::RendererVulkan::CreateImageView(VkImageView* _view,
                                        VkImage _image,
                                        VkFormat _format,
                                        VkImageAspectFlags _aspectMask)
{
  VkImageViewCreateInfo createInfo { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
  createInfo.flags = 0;
  createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  createInfo.image = _image;
  createInfo.format = _format;
  createInfo.subresourceRange = {};
  createInfo.subresourceRange.aspectMask = _aspectMask;
  createInfo.subresourceRange.levelCount = 1;
  createInfo.subresourceRange.baseMipLevel = 0;
  createInfo.subresourceRange.layerCount = 1;
  createInfo.subresourceRange.baseArrayLayer = 0;

  IVK_ASSERT(vkCreateImageView(context.device, &createInfo, context.alloc, _view),
             "Failed to create an image view");

  return true;
}
