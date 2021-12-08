
#ifndef ICE_RENDERING_VULKAN_RE_RENDERER_VULKAN_H_
#define ICE_RENDERING_VULKAN_RE_RENDERER_VULKAN_H_

#include "defines.h"

#include "rendering/vulkan/vulkan_context.h"
#include "rendering/renderer_backend.h"
#include "math/vector.h"

#include <vulkan/vulkan.h>

#include <vector>

class IvkRenderer : public reIceRendererBackend
{
private:
  reIvkContext context;

public:
  b8 Initialize() override;
  b8 Shutdown() override;
  b8 Render() override;

private:
  // Ensures that all desired layer and extension functionality is present in the created instance
  b8 CreateInstance();
  // Selects the first compatible GPU among those available in the system
  // Fills the selected GPU's information
  b8 ChoosePhysicalDevice();
  // Returns the first queue that satisfies the input flags
  u32 GetIndexForQueue(VkQueueFlags _flags);
  // Finds the first GPU queue that supports presentation
  u32 GetPresentQueue();
  // Tests the GPU for desired functionality
  b8 IsDeviceSuitable(const VkPhysicalDevice& _device);
  // Creates the vkDevice
  b8 CreateLogicalDevice();

  // Defines the descriptors and sets available for use
  b8 CreateDescriptorPool();
  // Creates a command pool
  b8 CreateCommandPool(b8 _createTransient = false);

  // Creates the swapchain, its images, and their views
  b8 CreateSwapchain();
  // Creates a simple forward renderpass
  b8 CreateRenderpass();
  // Creates the depth image and its view
  b8 CreateDepthImage();
  // Creates a frame buffer for each swapchain image
  b8 CreateFrameBuffers();

  // Creates the fences and semaphores for CPU/GPU synchronization
  b8 CreateSyncObjects();
  // Creates a command buffer for each frame
  b8 CreateCommandBuffers();

  b8 RecordCommandBuffer(u32 _commandIndex);

  // Platform =====
  // Retrieves all of the extensions the platform requires to render and present with Vulkan
  void GetPlatformExtensions(std::vector<const char*>& _extensions);
  // Creates a vendor-specific surface for display
  b8 CreateSurface();
  vec2U GetPlatformWindowExtents();

public:

  // Materials =====
  b8 CreateDescriptorSet();
  b8 CreatePipelinelayout();
  b8 CreatePipeline();

  // Buffers =====
  // Allocates a new block of memory on the GPU
  b8 CreateBuffer(IvkBuffer* _buffer,
                  u64 _size,
                  VkBufferUsageFlags _usage,
                  VkMemoryPropertyFlags _memoryProperties,
                  void* _data = nullptr);
  // Binds the buffer to the memory starting at the offset
  b8 BindBuffer(IvkBuffer* _buffer, u64 _offset);
  // Fills the given buffer with data
  b8 FillBuffer(IvkBuffer* _buffer, void* _data, u64 _size = 0);
  void DestroyBuffer(IvkBuffer* _buffer, b8 _freeMemory = false);

  // Images =====
  b8 CreateImage(reIvkImage* _image,
                 VkExtent2D _extents,
                 VkFormat _format,
                 VkImageUsageFlags _usage);
  b8 CreateImageView(VkImageView* _view,
                     VkImage _image,
                     VkFormat _format,
                     VkImageAspectFlags _aspectMask);
  b8 CreateImageSampler(reIvkImage* _image);

  b8 CreateShaderModule(VkShaderModule* _module, const char* _shader);

};

#endif // !define ICE_RENDERING_RE_RENDERER_VULKAN_H_
