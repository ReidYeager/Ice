
#include "defines.h"
#include "logger.h"

#include "rendering/vulkan/vulkan.h"
#include "rendering/vulkan/vulkan_defines.h"

#include <vector>

u32 FindMemoryType(const VkPhysicalDeviceMemoryProperties& _properties,
                   u32 _mask,
                   VkMemoryPropertyFlags _flags)
{
  for (u32 i = 0; i < _properties.memoryTypeCount; i++)
  {
    if (_mask & (1 << i) && (_properties.memoryTypes[i].propertyFlags & _flags) == _flags)
    {
      return i;
    }
  }

  IceLogError("Failed to find a suitable memory type -- Mask : %u, Flags : %u", _mask, _flags);
  return -1;
}

b8 Ice::RendererVulkan::CreateImage(Ice::Image* _image, void* _data)
{
  return true;
}

b8 Ice::RendererVulkan::CreateImage(Ice::IvkImage* _image,
                                    VkExtent2D _extents,
                                    VkFormat _format,
                                    VkImageUsageFlags _usage)
{
  // Creation =====
  VkImageCreateInfo createInfo { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
  createInfo.flags         = 0;
  createInfo.format        = _format;
  createInfo.usage         = _usage;
  createInfo.extent.width  = _extents.width;
  createInfo.extent.height = _extents.height;
  createInfo.extent.depth  = 1;
  createInfo.mipLevels     = 1;
  createInfo.arrayLayers   = 1;
  createInfo.imageType     = VK_IMAGE_TYPE_2D;
  createInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
  createInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
  createInfo.samples       = VK_SAMPLE_COUNT_1_BIT;

  IVK_ASSERT(vkCreateImage(context.device, &createInfo, context.alloc, &_image->image),
             "Failed to create an image");

  _image->format = _format;
  _image->extents = { _extents.width, _extents.height };

  // Image memory =====
  VkMemoryRequirements memoryReq;
  vkGetImageMemoryRequirements(context.device, _image->image, &memoryReq);

  VkMemoryAllocateInfo memoryAlloc { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
  memoryAlloc.allocationSize = memoryReq.size;

  // Search for an applicable memory type index
  memoryAlloc.memoryTypeIndex = FindMemoryType(context.gpu.memoryProperties,
                                               memoryReq.memoryTypeBits,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  IVK_ASSERT(vkAllocateMemory(context.device, &memoryAlloc, context.alloc, &_image->memory),
             "Failed to allocate image memory");

  // Binding =====
  IVK_ASSERT(vkBindImageMemory(context.device, _image->image, _image->memory, 0),
             "Failed to bind an image and its memory");

  return true;
}

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

b8 Ice::RendererVulkan::DestroyImage(Ice::IvkImage* _image)
{
  vkDestroyImage(context.device, _image->image, context.alloc);
  vkDestroyImageView(context.device, _image->view, context.alloc);
  vkFreeMemory(context.device, _image->memory, context.alloc);

  return true;
}
