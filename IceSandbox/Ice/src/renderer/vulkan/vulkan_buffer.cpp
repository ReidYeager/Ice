
#include "logger.h"
#include "renderer/vulkan/vulkan_buffer.h"

IvkBuffer::~IvkBuffer()
{
  if (buffer != VK_NULL_HANDLE)
  {
    IceLogError("Failed to free buffer of size %u", size);
  }
}

IvkBuffer::IvkBuffer(IceRenderContext* _rContext,
                     u64 _size,
                     VkBufferUsageFlags _usage,
                     VkMemoryPropertyFlags _memProperties,
                     bool _bind /*= true*/)
{
  size = PadBufferForGpu(_rContext, _size);
  usage = _usage;

  // Create the vkBuffer itself
  VkBufferCreateInfo createInfo {};
  createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.size = size;
  createInfo.usage = usage;
  createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  IVK_ASSERT(vkCreateBuffer(_rContext->device, &createInfo, _rContext->allocator, &buffer),
             "Failed to create buffer");

  // Allocate memory for the buffer on the GPU
  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(_rContext->device, buffer, &memRequirements);

  // Replace with a GPU memory allocator call?
  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =FindMemoryTypeIndex(_rContext,
                                                 memRequirements.memoryTypeBits,
                                                 _memProperties);

  IVK_ASSERT(vkAllocateMemory(_rContext->device, &allocInfo, _rContext->allocator, &memory),
             "Failed to allocate vert memory");

  if (_bind)
  {
    Bind(_rContext);
  }
}

void IvkBuffer::Free(IceRenderContext* _rContext)
{
  vkFreeMemory(_rContext->device, memory, _rContext->allocator);
  vkDestroyBuffer(_rContext->device, buffer, _rContext->allocator);

  buffer = VK_NULL_HANDLE;
}

void IvkBuffer::Bind(IceRenderContext* _rContext)
{
  vkBindBufferMemory(_rContext->device, buffer, memory, offset);
}

void IvkBuffer::Unbind(IceRenderContext* _rContext)
{
  // Remove the binding by replacing it with null
  vkBindBufferMemory(_rContext->device, buffer, VK_NULL_HANDLE, offset);
}

u32 IvkBuffer::FindMemoryTypeIndex(IceRenderContext* _rContext,
                                   u32 _supportedMemoryTypes,
                                   VkMemoryPropertyFlags _flags)
{
  const VkPhysicalDeviceMemoryProperties& props = _rContext->gpu.memProperties;

  for (u32 i = 0; i < props.memoryTypeCount; i++)
  {
    // if this memory type is supported and it has all the properties required
    if (_supportedMemoryTypes & (1 << i) &&
        (props.memoryTypes[i].propertyFlags & _flags) == _flags)
    {
      return i;
    }
  }

  IceLogInfo("Failed to find a suitable memory type");
  return -1;
}

u64 IvkBuffer::PadBufferForGpu(IceRenderContext* rContext, u64 _size)
{
  u64 alignment = rContext->gpu.properties.limits.minUniformBufferOffsetAlignment - 1;
  u64 alignedSize = _size;
  if (alignment > 0)
  {
    // Ceils the aligned size to the next alignment increment
    alignedSize = (alignedSize + alignment) & ~(alignment);
  }
  return alignedSize;
}
