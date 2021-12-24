
#include "defines.h"

#include "rendering/vulkan/vulkan_renderer.h"

u32 FindMemoryTypeIndex(reIvkContext* _context,
                        u32 _supportedMemoryTypes,
                        VkMemoryPropertyFlags _flags)
{
  const VkPhysicalDeviceMemoryProperties& props = _context->gpu.memoryProperties;

  for (u32 i = 0; i < props.memoryTypeCount; i++)
  {
    // if this memory type is supported and it has all the properties required
    if (_supportedMemoryTypes & (1 << i) &&
        (props.memoryTypes[i].propertyFlags & _flags) == _flags)
    {
      return i;
    }
  }

  IceLogError("Failed to find a suitable memory type");
  return -1;
}

b8 IvkRenderer::CreateBuffer(IvkBuffer* _buffer,
                             u64 _size,
                             VkBufferUsageFlags _usage,
                             VkMemoryPropertyFlags _memoryProperties,
                             void* _data /*= nullptr*/)
{
  // Pad size =====
  {
    u64 alignment = context.gpu.properties.limits.minUniformBufferOffsetAlignment - 1;
    if (alignment > 0)
    {
      _buffer->size = (_size + alignment) & ~alignment;
    }
    IceLogInfo("Buffer size : %lu -> %lu", _size, _buffer->size);
  }

  // Buffer creation =====
  VkBufferCreateInfo createInfo { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
  createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  createInfo.size = _buffer->size;
  createInfo.usage = _usage;

  IVK_ASSERT(vkCreateBuffer(context.device, &createInfo, context.alloc, &_buffer->buffer),
             "Failed to create buffer of size %llu", _buffer->size);

  // Memory allocation =====
  VkMemoryRequirements memoryReq;
  vkGetBufferMemoryRequirements(context.device, _buffer->buffer, &memoryReq);

  VkMemoryAllocateInfo allocInfo { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
  allocInfo.allocationSize = memoryReq.size;
  allocInfo.memoryTypeIndex = FindMemoryTypeIndex(&context,
                                                  memoryReq.memoryTypeBits,
                                                  _memoryProperties);

  IVK_ASSERT(vkAllocateMemory(context.device, &allocInfo, context.alloc, &_buffer->memory),
             "Failed to allocate buffer memory");

  _buffer->offset = 0;

  // Should auto-binding even be in place?
  BindBuffer(_buffer, 0);

  // Fill the buffer =====
  if (_data != nullptr)
  {
    ICE_ATTEMPT(FillBuffer(_buffer, _data, _size));
  }

  return true;
}

b8 IvkRenderer::BindBuffer(IvkBuffer* _buffer, u64 _offset)
{
  // Bind the buffer and memory =====
  IVK_ASSERT(vkBindBufferMemory(context.device, _buffer->buffer, _buffer->memory, _offset),
             "Failed to bind a buffer and memory");
  return true;
}

b8 IvkRenderer::FillBuffer(IvkBuffer* _buffer, void* _data, u64 _size /*= 0*/, u64 _offset /*= 0*/)
{
  void* mappedMemory;
  // Use the buffer's whole size if undefined
  u64 usedSize = _size == 0 ? _buffer->size : _size;

  vkMapMemory(context.device, _buffer->memory, _buffer->offset + _offset, usedSize, 0, &mappedMemory);
  rePlatform.MemCopy(_data, mappedMemory, usedSize);
  vkUnmapMemory(context.device, _buffer->memory);

  return true;
}

void IvkRenderer::DestroyBuffer(const IvkBuffer* _buffer, b8 _freeMemory /*= false*/)
{
  vkDestroyBuffer(context.device, _buffer->buffer, context.alloc);
  if (_freeMemory)
  {
    vkFreeMemory(context.device, _buffer->memory, context.alloc);
  }
}
