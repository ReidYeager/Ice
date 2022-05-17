
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
                                           u64 _elementSize,
                                           u32 _elementCount,
                                           Ice::BufferMemoryUsageFlags _usage)
{
  if (_elementSize * _elementCount == 0)
  {
    IceLogError("Can not create buffer with size 0\n> Element size %llu, Count : %u",
                _elementSize,
                _elementCount);
    ICE_BREAK;
    return false;
  }

  *_outBuffer = { _elementSize, PadBufferSize(_elementSize, _usage), _elementCount, _usage };

  // Buffer =====
  VkBufferCreateInfo createInfo { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
  createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  createInfo.size = _outBuffer->padElementSize * _outBuffer->count;

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
  if (_usage & Ice::Buffer_Memory_Transfer_Src)
  {
    createInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  }
  if (_usage & Ice::Buffer_Memory_Transfer_Dst)
  {
    createInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  }

  IVK_ASSERT(vkCreateBuffer(context.device, &createInfo, context.alloc, &_outBuffer->vulkan.buffer),
             "Failed to create buffer : size %llu", _outBuffer->padElementSize * _outBuffer->count);

  // Memory =====
  VkMemoryRequirements bufferMemRequirements;
  vkGetBufferMemoryRequirements(context.device, _outBuffer->vulkan.buffer, &bufferMemRequirements);

  VkMemoryAllocateInfo allocInfo { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
  allocInfo.allocationSize = bufferMemRequirements.size;
  allocInfo.memoryTypeIndex = GetMemoryTypeIndex(&context,
                                                 bufferMemRequirements.memoryTypeBits,
                                                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

  IVK_ASSERT(vkAllocateMemory(context.device, &allocInfo, context.alloc, &_outBuffer->vulkan.memory),
             "Failed to allocate buffer memory : size %llu",
             _outBuffer->padElementSize * _outBuffer->count);

  // Bind =====
  IVK_ASSERT(vkBindBufferMemory(context.device,
                                _outBuffer->vulkan.buffer,
                                _outBuffer->vulkan.memory, 0),
             "Failed to bind buffer and memory");

  return true;
}

void Ice::RendererVulkan::DestroyBufferMemory(Ice::Buffer* _buffer)
{
  if (_buffer == nullptr ||
      _buffer->vulkan.buffer == nullptr ||
      _buffer->padElementSize * _buffer->count == 0)
    return;

  vkDeviceWaitIdle(context.device);
  vkDestroyBuffer(context.device, _buffer->vulkan.buffer, context.alloc);
  vkFreeMemory(context.device, _buffer->vulkan.memory, context.alloc);
}

b8 Ice::RendererVulkan::PushDataToBuffer(void* _data, const Ice::BufferSegment _segmentInfo)
{
  if (_segmentInfo.buffer == nullptr)
  {
    IceLogError("No buffer given to data push segment. Aborting data push.");
    return false;
  }

  // Using char* to index one byte at a time
  char* mappedGpuMemory;
  char* cpuMemory = (char*)_data;

  u64 stride = _segmentInfo.buffer->padElementSize;

  u64 bufferOffset = _segmentInfo.startIndex * stride; // GPU byte index
  u64 bufferSize = _segmentInfo.count * stride; // Total byte range being accessed

  u64 copyElementSize = _segmentInfo.elementSize; // Bytes of each element to copy
  u64 elementOffset = _segmentInfo.offset; // Offset into the element to start copy

  // If zero, use buffer elements' full size
  if (!copyElementSize)
    copyElementSize = _segmentInfo.buffer->elementSize;

  IVK_ASSERT(vkMapMemory(context.device,
                         _segmentInfo.buffer->vulkan.memory,
                         bufferOffset,
                         bufferSize,
                         0,
                         (void**)&mappedGpuMemory),
             "Failed to map buffer memory\n> Offset %llu, Size %llu", bufferOffset, bufferSize);

  // Copy copyElementSize bytes at a time into each element for count elements
  for (u32 i = 0; i < _segmentInfo.count; i++)
  {
    Ice::MemoryCopy((void*)(cpuMemory + (copyElementSize * i)), // i-th array element
                    (void*)(mappedGpuMemory + (stride * i) + elementOffset), // i-th padded GPU element
                    copyElementSize);
  }

  vkUnmapMemory(context.device, _segmentInfo.buffer->vulkan.memory);

  return true;
}

u64 Ice::RendererVulkan::PadBufferSize(u64 _size, Ice::BufferMemoryUsageFlags _usage)
{
  u64 alignment = 0;
  if (_usage & Ice::Buffer_Memory_Shader_Read)
  {
    alignment = context.gpu.properties.limits.minUniformBufferOffsetAlignment - 1;
  }
  else
  {
    alignment = context.gpu.properties.limits.minStorageBufferOffsetAlignment - 1;
  }
  return (_size + alignment) & ~alignment;
}
