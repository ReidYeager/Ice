
#include "defines.h"

#include "rendering/vulkan/re_renderer_vulkan.h"
#include "math/vector.h"

#include <vulkan/vulkan.h>

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

b8 reIvkRenderer::CreateImage(reIvkImage* _image,
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

b8 reIvkRenderer::CreateImageView(reIvkImage* _image, VkImageAspectFlags _aspectMask)
{
  VkImageViewCreateInfo createInfo { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
  createInfo.flags = 0;
  createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  createInfo.image = _image->image;
  createInfo.format = _image->format;
  createInfo.subresourceRange.aspectMask = _aspectMask;
  createInfo.subresourceRange.levelCount = 1;
  createInfo.subresourceRange.baseMipLevel = 0;
  createInfo.subresourceRange.layerCount = 1;
  createInfo.subresourceRange.baseArrayLayer = 0;
  createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

  IVK_ASSERT(vkCreateImageView(context.device, &createInfo, context.alloc, &_image->view),
             "Failed to create an image view");

  return true;
}

b8 reIvkRenderer::CreateImageSampler(reIvkImage* _image)
{
  VkSamplerCreateInfo createInfo { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
  createInfo.magFilter = VK_FILTER_LINEAR;
  createInfo.minFilter = VK_FILTER_LINEAR;
  createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

  createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  createInfo.unnormalizedCoordinates = VK_FALSE;
  createInfo.compareEnable = VK_FALSE;
  createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  createInfo.mipLodBias = 0.0f;
  createInfo.minLod = 0.0f;
  createInfo.maxLod = 0.0f;

  IVK_ASSERT(vkCreateSampler(context.device, &createInfo, context.alloc, &_image->sampler),
             "Failed to create an image sampler");

  return true;
}
