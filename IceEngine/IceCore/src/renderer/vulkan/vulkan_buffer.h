
#ifndef ICE_RENDERER_BUFFER_H_
#define ICE_RENDERER_BUFFER_H_

#include "defines.h"
#include "renderer/renderer_backend_context.h"

#include <vulkan/vulkan.h>

#include <vector>

class IvkBuffer
{
  // TODO : Add alignment to better utilize GPU
private:
  VkBuffer m_buffer = VK_NULL_HANDLE;
  VkDeviceMemory m_memory = VK_NULL_HANDLE;
  VkBufferUsageFlags m_usage;
  u32 m_size;
  u32 m_offset = 0;
  bool m_isBound = false;

public:
  IvkBuffer(IceRenderContext* rContext,
            u32 _size,
            VkBufferUsageFlags m_usage,
            VkMemoryPropertyFlags _memProperties,
            bool _bind = true);
  ~IvkBuffer();

  void FreeBuffer(IceRenderContext* rContext);
  void Bind(IceRenderContext* rContext);
  void Unbind(IceRenderContext* rContext);

  u32 GetSize() const { return m_size; }
  VkBufferUsageFlags GetUsage() const { return m_usage; }
  VkBuffer GetBuffer() const { return m_buffer; }
  VkBuffer* GetBufferPtr() { return &m_buffer; }
  VkDeviceMemory GetMemory() const { return m_memory; }

  // TODO : Move to vkMemory allocator
  u32 FindMemoryTypeIndex(IceRenderContext* _rContext, u32 _mask, VkMemoryPropertyFlags _flags);
};

// TODO : Add specialized implementations for vertex, index, and uniform buffers?

#endif // !RENDERER_BUFFER_H
