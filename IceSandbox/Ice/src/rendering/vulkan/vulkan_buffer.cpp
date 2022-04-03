
#include "defines.h"

#include "rendering/vulkan/vulkan.h"

u32 GetMemoryTypeIndex(Ice::VulkanContext* _context,
                       u32 _supportedMemoryTypes,
                       VkMemoryPropertyFlags _flags)
{
  const VkPhysicalDeviceMemoryProperties& properties = _context->gpu.memoryProperties;

  for (u32 i = 0; i < properties.memoryTypeCount; i++)
  {
    if (_supportedMemoryTypes & (1 << i) &&
        (properties.memoryTypes[i].propertyFlags & _flags) == _flags)
    {
      return i;
    }
  }

  IceLogError("No suitable memory type is supported for this buffer");
  return ICE_NULL_UINT;
}

b8 Ice::RendererVulkan::CreateBufferMemory(Ice::Buffer* _outBuffer,
                                           u64 _size,
                                           Ice::BufferMemoryUsageFlags _usage)
{
  // Pad size =====
  u64 alignment = 0;
  if (_usage & Ice::Buffer_Memory_Shader_Read)
  {
    alignment = context.gpu.properties.limits.minUniformBufferOffsetAlignment - 1;
  }
  else
  {
    alignment = context.gpu.properties.limits.minStorageBufferOffsetAlignment - 1;
  }
  _outBuffer->size = (_size + alignment) & ~alignment;

  // Buffer =====
  VkBufferCreateInfo createInfo { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
  createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  createInfo.size = _outBuffer->size;

  if (_usage & Ice::Buffer_Memory_Shader_Read)
  {
    createInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  }
  if (_usage & Ice::Buffer_Memory_Vertex)
  {
    createInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  }
  if (_usage & Ice::Buffer_Memory_Index)
  {
    createInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  }

  IVK_ASSERT(vkCreateBuffer(context.device, &createInfo, context.alloc, &_outBuffer->ivkBuffer),
             "Failed to create buffer : size %llu", _outBuffer->size);

  // Memory =====
  VkMemoryRequirements bufferMemRequirements;
  vkGetBufferMemoryRequirements(context.device, _outBuffer->ivkBuffer, &bufferMemRequirements);

  VkMemoryAllocateInfo allocInfo { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
  allocInfo.allocationSize = bufferMemRequirements.size;
  allocInfo.memoryTypeIndex = GetMemoryTypeIndex(&context,
                                                 bufferMemRequirements.memoryTypeBits,
                                                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

  IVK_ASSERT(vkAllocateMemory(context.device, &allocInfo, context.alloc, &_outBuffer->ivkMemory),
             "Failed to allocate buffer memory : size %llu", _outBuffer->size);

  // Bind =====
  IVK_ASSERT(vkBindBufferMemory(context.device, _outBuffer->ivkBuffer, _outBuffer->ivkMemory, 0),
             "Failed to bind buffer and memory");

  return true;
}

void Ice::RendererVulkan::DestroyBufferMemory(Ice::Buffer* _buffer)
{
  vkDeviceWaitIdle(context.device);
  vkDestroyBuffer(context.device, _buffer->ivkBuffer, context.alloc);
  vkFreeMemory(context.device, _buffer->ivkMemory, context.alloc);
}

b8 Ice::RendererVulkan::PushDataToBuffer(void* _data,
                                         const Ice::Buffer* _buffer,
                                         const Ice::BufferSegment* _segmentInfo /*= nullptr*/)
{
  if (_segmentInfo != nullptr && _segmentInfo->ivkBuffer != _buffer->ivkBuffer)
  {
    IceLogError("Buffer does not match segment's buffer. Aborting data push.");
    return false;
  }

  void* mappedGpuMemory;

  u64 offset = _segmentInfo == nullptr ? 0 : _segmentInfo->offset;
  u64 size = _segmentInfo == nullptr ? _buffer->size : _segmentInfo->size;

  IVK_ASSERT(vkMapMemory(context.device, _buffer->ivkMemory, offset, size, 0, &mappedGpuMemory),
             "Failed to map buffer memory\n> Offset %llu, Size %llu", offset, size);
  Ice::MemoryCopy(_data, mappedGpuMemory, size);
  vkUnmapMemory(context.device, _buffer->ivkMemory);

  return true;
}

b8 Ice::RendererVulkan::QueueDataToBuffer(void* _data,
                                          const Ice::Buffer* _buffer,
                                          const Ice::BufferSegment* _segmentInfo /*= nullptr*/)
{
  if (_segmentInfo != nullptr && _segmentInfo->ivkBuffer != _buffer->ivkBuffer)
  {
    IceLogError("Buffer does not match segment's buffer. Aborting data push.");
    return false;
  }

  // TODO : Implement buffer data queue
}

b8 FlushBufferQueue()
{
  return true;
}

