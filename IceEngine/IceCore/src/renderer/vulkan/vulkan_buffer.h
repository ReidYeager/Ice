
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
  // Creates the buffer and its memory
  IvkBuffer(IceRenderContext* rContext,
            u64 _size,
            VkBufferUsageFlags m_usage,
            VkMemoryPropertyFlags _memProperties,
            bool _bind = true);
  // Warns if the buffer has not been freed
  // Cannot do so itself because it does not have access to the render context
  ~IvkBuffer();

  // Frees the buffer and its memory
  void Free(IceRenderContext* rContext);
  // Binds the buffer to its memory
  void Bind(IceRenderContext* rContext);
  // Removes any existing binding
  void Unbind(IceRenderContext* rContext);

  // Getters
  u64 GetSize() const { return size; }
  VkBufferUsageFlags GetUsage() const { return usage; }
  VkBuffer GetBuffer() const { return buffer; }
  VkBuffer* GetBufferPtr() { return &buffer; }
  VkDeviceMemory GetMemory() const { return memory; }

  // TODO : Move to a vkMemory allocator
  u32 FindMemoryTypeIndex(IceRenderContext* _rContext, u32 _mask, VkMemoryPropertyFlags _flags);

private:
  // Pads the input to the next multiple of the GPU's minimum offset increment
  u64 PadBufferForGpu(IceRenderContext* rContext, u64 _size);

};

#endif // !RENDERER_BUFFER_H
