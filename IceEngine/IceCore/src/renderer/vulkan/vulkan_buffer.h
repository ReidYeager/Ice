
#ifndef ICE_RENDERER_VULKAN_VULKAN_BUFFER_H_
#define ICE_RENDERER_VULKAN_VULKAN_BUFFER_H_

#include "defines.h"
#include "renderer/renderer_backend_context.h"

#include <vulkan/vulkan.h>

#include <vector>

class IvkBuffer
{
  // TODO : Add alignment to better utilize GPU
private:
  VkBuffer buffer = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  VkBufferUsageFlags usage;
  u64 size;
  u64 offset = 0;
  bool isBound = false;

public:
  IvkBuffer(IceRenderContext* rContext,
            u64 _size,
            VkBufferUsageFlags m_usage,
            VkMemoryPropertyFlags _memProperties,
            bool _bind = true);
  ~IvkBuffer();

  void FreeBuffer(IceRenderContext* rContext);
  void Bind(IceRenderContext* rContext);
  void Unbind(IceRenderContext* rContext);

  u64 GetSize() const { return size; }
  VkBufferUsageFlags GetUsage() const { return usage; }
  VkBuffer GetBuffer() const { return buffer; }
  VkBuffer* GetBufferPtr() { return &buffer; }
  VkDeviceMemory GetMemory() const { return memory; }

  // TODO : Move to vkMemory allocator
  u32 FindMemoryTypeIndex(IceRenderContext* _rContext, u32 _mask, VkMemoryPropertyFlags _flags);
};

#endif // !RENDERER_BUFFER_H
