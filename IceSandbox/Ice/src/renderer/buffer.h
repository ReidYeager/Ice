
#ifndef ICE_RENDERER_BUFFER_H_
#define ICE_RENDERER_BUFFER_H_

#include "defines.h"

#include "renderer/vulkan/vulkan_buffer.h"
#include "renderer/renderer_backend_context.h"
#include "renderer/shader_parameters.h"

typedef IvkBuffer* IceBuffer;

// Creates a buffer on the GPU and fills it with data
IceBuffer CreateAndFillBuffer(IceRenderContext* rContext, const void* _data, VkDeviceSize _size, VkBufferUsageFlags _usage);
IceBuffer CreateBuffer(IceRenderContext* rContext, VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _memProperties);
IceBuffer CreateStagingBuffer(IceRenderContext* rContext, VkDeviceSize _size);
void FillShaderBuffer(IceRenderContext* rContext, IceBuffer _buffer, const void* _data, IceShaderBufferParameterFlags _flags, IceShaderBufferParameterFlags _shaderParams);
void FillBuffer(IceRenderContext* rContext, VkDeviceMemory _mem, const void* _data, VkDeviceSize _size);
void FillBuffer(IceRenderContext* rContext, IceBuffer _buffer, const void* _data, VkDeviceSize _size);
void CopyBuffer(IceRenderContext* rContext, VkBuffer _src, VkBuffer _dst, VkDeviceSize _size, VkDeviceSize _offsets = 0);
void DestroyBuffer(IceRenderContext* rContext, VkBuffer _buffer, VkDeviceMemory _memory);

#endif // !define ICE_RENDERER_BUFFER_H_

