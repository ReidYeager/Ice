
#ifndef ICE_RENDERER_BUFFER_H_
#define ICE_RENDERER_BUFFER_H_

#include "defines.h"

#include "renderer/vulkan/vulkan_buffer.h"
#include "renderer/renderer_backend_context.h"

// Creates a buffer on the GPU and fills it with data
IvkBuffer* CreateAndFillBuffer(IceRenderContext* rContext, const void* _data, VkDeviceSize _size, VkBufferUsageFlags _usage);
IvkBuffer* CreateBuffer(
  IceRenderContext* rContext, VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _memProperties);
void FillBuffer(IceRenderContext* rContext, VkDeviceMemory _mem, const void* _data, VkDeviceSize _size);
void FillBuffer(IceRenderContext* rContext, IvkBuffer* _buffer, const void* _data, VkDeviceSize _size);
void CopyBuffer(IceRenderContext* rContext, VkBuffer _src, VkBuffer _dst, VkDeviceSize _size);
void DestroyBuffer(IceRenderContext* rContext, VkBuffer _buffer, VkDeviceMemory _memory);

struct IceBuffer_T
{
  void* data;
  IceDeviceSize size;
  IceDeviceSize offset;
  b8 isBound;
};

typedef IceBuffer_T* IceBuffer;

#endif // !define ICE_RENDERER_BUFFER_H_

