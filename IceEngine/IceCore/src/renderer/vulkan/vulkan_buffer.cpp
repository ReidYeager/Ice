
#include "logger.h"
#include "renderer/vulkan/vulkan_buffer.h"

IvkBuffer::~IvkBuffer()
{
  if (buffer != VK_NULL_HANDLE)
  {
    LogError("Failed to free buffer of size %u", size);
  }
}

//size_t PadBufferForGpu(IceRenderContext* rContext, size_t _original)
//{
//  size_t alignment = rContext->gpu.properties.limits.minUniformBufferOffsetAlignment;
//  size_t alignedSize = _original;
//  if (alignment > 0)
//  {
//    alignedSize = (alignedSize + alignment - 1) & ~(alignment - 1);
//  }
//  return alignedSize;
//}

IvkBuffer::IvkBuffer(IceRenderContext* _rContext,
                     u64 _size,
                     VkBufferUsageFlags _usage,
                     VkMemoryPropertyFlags _memProperties,
                     bool _bind /*= true*/)
{
  // Align data


  size = _size;
  usage = _usage;

  VkBufferCreateInfo createInfo {};
  createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.size = size;
  createInfo.usage = usage;
  createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  IVK_ASSERT(vkCreateBuffer(_rContext->device, &createInfo, _rContext->allocator, &buffer),
             "Failed to create buffer");

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(_rContext->device, buffer, &memRequirements);

  // TODO : Replace with a GPU memory allocator call
  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      FindMemoryTypeIndex(_rContext, memRequirements.memoryTypeBits, _memProperties);

  IVK_ASSERT(vkAllocateMemory(_rContext->device, &allocInfo, _rContext->allocator, &memory),
             "Failed to allocate vert memory");

  if (_bind)
  {
    Bind(_rContext);
  }
}

void IvkBuffer::FreeBuffer(IceRenderContext* _rContext)
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

}

u32 IvkBuffer::FindMemoryTypeIndex(IceRenderContext* _rContext, u32 _mask, VkMemoryPropertyFlags _flags)
{
  const VkPhysicalDeviceMemoryProperties& props = _rContext->gpu.memProperties;

  for (u32 i = 0; i < props.memoryTypeCount; i++)
  {
    if (_mask & (1 << i) && (props.memoryTypes[i].propertyFlags & _flags) == _flags)
    {
      return i;
    }
  }

  LogInfo("Failed to find a suitable memory type");
  return -1;
}
