
#include "defines.h"

#include "rendering/vulkan/vulkan_renderer.h"

#include "math/vector.h"
#include "platform/file_system.h"

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

b8 IvkRenderer::CreateImage(IvkImage* _image,
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

void IvkRenderer::DestroyImage(const IvkImage* _image)
{
  vkDestroyImage(context.device, _image->image, context.alloc);
  vkDestroyImageView(context.device, _image->view, context.alloc);
  vkDestroySampler(context.device, _image->sampler, context.alloc);
  vkFreeMemory(context.device, _image->memory, context.alloc);
}

b8 IvkRenderer::CreateImageView(VkImageView* _view,
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

b8 IvkRenderer::CreateImageSampler(IvkImage* _image)
{
  VkSamplerCreateInfo createInfo { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
  createInfo.magFilter = VK_FILTER_LINEAR;
  createInfo.minFilter = VK_FILTER_LINEAR;
  createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  createInfo.addressModeV = createInfo.addressModeU;
  createInfo.addressModeW = createInfo.addressModeU;

  createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
  createInfo.unnormalizedCoordinates = VK_FALSE;
  createInfo.compareEnable = VK_FALSE;
  createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  createInfo.mipLodBias = 0.0f;
  createInfo.minLod = 0.0f;
  createInfo.maxLod = 1.0f;
  createInfo.maxAnisotropy = 1.0f;

  IVK_ASSERT(vkCreateSampler(context.device, &createInfo, context.alloc, &_image->sampler),
             "Failed to create an image sampler");

  return true;
}

u32 IvkRenderer::CreateTexture(const char* _directory)
{
  // Look for existing texture =====
  u32 index = 0;

  for (const auto& t : textures)
  {
    if (t.directory.compare(_directory) == 0)
    {
      return index;
    }
    index++;
  }

  // Create new texture =====
  IvkTexture tex;
  index = textures.size();

  std::string dir = ICE_RESOURCE_TEXTURE_DIR;
  dir.append(_directory);

  tex.directory = dir;

  ICE_ATTEMPT(PopulateTextureData(&tex));

  textures.push_back(tex);

  return index;
}

b8 IvkRenderer::PopulateTextureData(IvkTexture* _texture)
{
  IvkImage* image = &_texture->image;

  // Load the image file =====
  int width, height;
  void* imageSource = FileSystem::LoadImageFile(_texture->directory.c_str(), width, height);
  VkDeviceSize imageSize = 4 * (VkDeviceSize)width * (VkDeviceSize)height;

  // Create texture buffer =====
  IvkBuffer transferBuffer;
  CreateBuffer(&transferBuffer,
               imageSize,
               VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
               imageSource);

  // Create an IceImage =====
  VkExtent2D extents = { (u32)width, (u32)height };
  ICE_ATTEMPT(CreateImage(image,
                          extents,
                          VK_FORMAT_R8G8B8A8_UNORM,
                          VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT));
  ICE_ATTEMPT(CreateImageView(&image->view,
                              image->image,
                              image->format,
                              VK_IMAGE_ASPECT_COLOR_BIT));
  ICE_ATTEMPT(CreateImageSampler(image));

  // Fill the image info =====
  TransitionImageLayout(image, false, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
  CopyBufferToImage(&transferBuffer, image);
  TransitionImageLayout(image, true, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

  DestroyBuffer(&transferBuffer, true);

  return true;
}

void IvkRenderer::TransitionImageLayout(IvkImage* _image,
                                        b8 _forSampling,
                                        VkPipelineStageFlagBits _shaderStage)
{
  VkCommandBuffer command = BeginSingleTimeCommand(context.graphicsCommandPool);

  VkImageMemoryBarrier memBarrier { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
  memBarrier.oldLayout = _image->layout;
  memBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  memBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  memBarrier.image = _image->image;
  memBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  memBarrier.subresourceRange.levelCount = 1;
  memBarrier.subresourceRange.baseMipLevel = 0;
  memBarrier.subresourceRange.layerCount = 1;
  memBarrier.subresourceRange.baseArrayLayer = 0;

  VkPipelineStageFlagBits srcStage, dstStage;

  if (!_forSampling)
  {
    memBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    memBarrier.srcAccessMask = 0;
    memBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else
  {
    memBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    memBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    memBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dstStage = _shaderStage;
  }

  vkCmdPipelineBarrier(command, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &memBarrier);
  EndSingleTimeCommand(command, context.graphicsCommandPool, context.graphicsQueue);

  _image->layout = memBarrier.newLayout;
}

void IvkRenderer::CopyBufferToImage(IvkBuffer* _buffer, IvkImage* _image)
{
  VkCommandBuffer command = BeginSingleTimeCommand(context.transientCommandPool);

  VkBufferImageCopy copyRegion {};
  copyRegion.bufferOffset = 0;
  copyRegion.bufferRowLength = _image->extents.width;
  copyRegion.bufferImageHeight = _image->extents.height;

  copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  copyRegion.imageSubresource.mipLevel = 0;
  copyRegion.imageSubresource.layerCount = 1;
  copyRegion.imageSubresource.baseArrayLayer = 0;

  copyRegion.imageOffset = { 0, 0, 0 };
  copyRegion.imageExtent = { _image->extents.width, _image->extents.height, 1 };

  vkCmdCopyBufferToImage(command,
                         _buffer->buffer,
                         _image->image,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         1,
                         &copyRegion);

  EndSingleTimeCommand(command, context.transientCommandPool, context.transientQueue);
}
