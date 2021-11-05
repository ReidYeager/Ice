
#include "defines.h"

#include "renderer/buffer.h"

size_t PadBufferForGpu(IceRenderContext* rContext, size_t _original)
{
  size_t alignment = rContext->gpu.properties.limits.minUniformBufferOffsetAlignment;
  size_t alignedSize = _original;
  if (alignment > 0)
  {
    alignedSize = (alignedSize + alignment - 1) & ~(alignment - 1);
  }
  return alignedSize;
}

IceBuffer CreateAndFillBuffer(
    IceRenderContext* rContext, const void* _data, VkDeviceSize _size, VkBufferUsageFlags _usage)
{
  IceBuffer stagingBuffer = CreateBuffer(rContext,
                                         _size,
                                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  FillBuffer(rContext, stagingBuffer->GetMemory(), _data, _size);

  IceBuffer buffer = CreateBuffer(rContext,
                                  _size,
                                  _usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  CopyBuffer(rContext, stagingBuffer->GetBuffer(), buffer->GetBuffer(), _size);

  stagingBuffer->Free(rContext);

  return buffer;
}

IceBuffer CreateBuffer(IceRenderContext* rContext,
                        VkDeviceSize _size,
                        VkBufferUsageFlags _usage,
                        VkMemoryPropertyFlags _memProperties)
{
  IceBuffer buffer = new IvkBuffer(
      rContext, static_cast<u32>(_size), _usage|VK_BUFFER_USAGE_TRANSFER_DST_BIT, _memProperties);

  return buffer;
}

void FillBuffer(
    IceRenderContext* rContext, VkDeviceMemory _mem, const void* _data, VkDeviceSize _size)
{
  void* tmpData;
  vkMapMemory(rContext->device, _mem, 0, _size, 0, &tmpData);
  memcpy(tmpData, _data, static_cast<size_t>(_size));
  vkUnmapMemory(rContext->device, _mem);
}

void FillBuffer(
  IceRenderContext* rContext, IceBuffer _buffer, const void* _data, VkDeviceSize _size)
{
  IceBuffer stagingBuffer = CreateBuffer(rContext,
                                         _size,
                                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  FillBuffer(rContext, stagingBuffer->GetMemory(), _data, _size);

  CopyBuffer(rContext, stagingBuffer->GetBuffer(), _buffer->GetBuffer(), _size);

  stagingBuffer->Free(rContext);
}

void FillShaderBuffer(IceRenderContext* rContext,
                      IceBuffer _buffer,
                      const void* _data,
                      IceShaderBufferParameterFlags _flags,
                      IceShaderBufferParameterFlags _shaderParams)
{
  if ((_flags & _shaderParams) != _flags)
  {
    IceLogError("Can not fill buffer at %lu, the buffer does not use those flags", _flags);
    return;
  }

  u64 offset = 0;
  u64 size = 0;

  // Find the first contiguous group of set bits
  for (u64 position = 0, value = 0; !size || value; position++)
  {
    value = (_flags >> position) & 1;

    size += value;
    offset += !size ? ((_shaderParams >> position) & 1) : 0;

  }

  if (size == 0)
  {
    IceLogError("Can not fill shader buffer with size 0");
    return;
  }

  offset *= 16;
  size *= 16; // Convert to # bytes

  // Stage the data
  IceBuffer stagingBuffer = CreateBuffer(rContext,
                                         size,
                                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  void* tmpData;
  vkMapMemory(rContext->device, stagingBuffer->GetMemory(), offset, size, 0, &tmpData);
  memcpy(tmpData, _data, static_cast<size_t>(size));
  vkUnmapMemory(rContext->device, stagingBuffer->GetMemory());

  // Copy staged buffer
  CopyBuffer(rContext, stagingBuffer->GetBuffer(), _buffer->GetBuffer(), size, offset);
  stagingBuffer->Free(rContext);
}

void CopyBuffer(IceRenderContext* rContext,
                VkBuffer _src,
                VkBuffer _dst,
                VkDeviceSize _size,
                VkDeviceSize _offsets /*= 0*/)
{
  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = rContext->transientCommandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer transferCommand;
  IVK_ASSERT(vkAllocateCommandBuffers(rContext->device, &allocInfo, &transferCommand),
    "Failed to create transient command buffer");

  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vkBeginCommandBuffer(transferCommand, &beginInfo);

  VkBufferCopy region = {};
  region.size = _size;
  region.dstOffset = _offsets;
  region.srcOffset = _offsets;

  vkCmdCopyBuffer(transferCommand, _src, _dst, 1, &region);

  vkEndCommandBuffer(transferCommand);

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &transferCommand;

  vkQueueSubmit(rContext->transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(rContext->transferQueue);

  vkFreeCommandBuffers(rContext->device, rContext->transientCommandPool, 1, &transferCommand);
}

void DestroyBuffer(IceRenderContext* rContext, VkBuffer _buffer, VkDeviceMemory _memory)
{
  vkFreeMemory(rContext->device, _memory, rContext->allocator);
  vkDestroyBuffer(rContext->device, _buffer, rContext->allocator);
}
