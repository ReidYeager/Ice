
#ifndef RENDERER_BUFFER_H
#define RENDERER_BUFFER_H 1

#include "defines.h"
#include "renderer/backend_context.h"

#include <vulkan/vulkan.h>
#include <vector>

// NOTE : I really don't like this implementation; especially as it stands now.

// TODO : Make API agnostic
class IceBuffer
{
  // TODO : Add alignment to better utilize GPU
private:
  VkBuffer m_buffer;
  VkDeviceMemory m_memory;
  VkBufferUsageFlags m_usage;
  u32 m_size;
  bool m_isBound = false;
  u32 m_offset = 0;

public:
  //IceBuffer();
  ~IceBuffer();

  void AllocateBuffer(u32 _size, VkBufferUsageFlags m_usage, VkMemoryPropertyFlags _memProperties,
                      bool _bind = true);
  void FreeBuffer();
  void Bind();
  void Unbind();

  u32 GetSize() const { return m_size; }
  VkBufferUsageFlags GetUsage() const { return m_usage; }
  VkBuffer GetBuffer() const { return m_buffer; }
  VkBuffer* GetBufferPtr() { return &m_buffer; }
  VkDeviceMemory GetMemory() const { return m_memory; }

  // TODO : Move to vkMemory allocator
  u32 FindMemoryTypeIndex(u32 _mask, VkMemoryPropertyFlags _flags);

};

// TODO : Add specialized implementations for vertex, index, and uniform buffers

#endif // !RENDERER_BUFFER_H
