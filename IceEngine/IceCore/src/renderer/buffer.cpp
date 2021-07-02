
#include "buffer.h"

#include "logger.h"

IceBuffer::~IceBuffer()
{
  if (m_buffer != VK_NULL_HANDLE)
    FreeBuffer();
}

void IceBuffer::AllocateBuffer(
    u32 _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _memProperties,
    bool _bind /*= true*/)
{
  // Align data

  m_size = _size;
  m_usage = _usage;

  VkBufferCreateInfo createInfo {};
  createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.size = m_size;
  createInfo.usage = m_usage;
  createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  ICE_ASSERT(vkCreateBuffer(rContext.device, &createInfo, rContext.allocator, &m_buffer),
             "Failed to create buffer");

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(rContext.device, m_buffer, &memRequirements);

  // TODO : Replace with a GPU memory allocator call
  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = FindMemoryTypeIndex(memRequirements.memoryTypeBits, _memProperties);

  ICE_ASSERT(vkAllocateMemory(rContext.device, &allocInfo, rContext.allocator, &m_memory),
             "Failed to allocate vert memory");

  if (_bind)
  {
    Bind();
  }
}

void IceBuffer::FreeBuffer()
{
  vkFreeMemory(rContext.device, m_memory, rContext.allocator);
  vkDestroyBuffer(rContext.device, m_buffer, rContext.allocator);

  m_buffer = VK_NULL_HANDLE;
}

void IceBuffer::Bind()
{
  vkBindBufferMemory(rContext.device, m_buffer, m_memory, m_offset);
}

void IceBuffer::Unbind()
{

}

u32 IceBuffer::FindMemoryTypeIndex(u32 _mask, VkMemoryPropertyFlags _flags)
{
  const VkPhysicalDeviceMemoryProperties& props = rContext.gpu.memProperties;

  for (u32 i = 0; i < props.memoryTypeCount; i++)
  {
    if (_mask & (1 << i) && (props.memoryTypes[i].propertyFlags & _flags) == _flags)
    {
      return i;
    }
  }

  IcePrint("Failed to find a suitable memory type");
  return -1;
}
