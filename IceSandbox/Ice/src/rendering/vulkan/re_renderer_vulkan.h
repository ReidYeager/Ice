
#ifndef ICE_RENDERING_VULKAN_RE_RENDERER_VULKAN_H_
#define ICE_RENDERING_VULKAN_RE_RENDERER_VULKAN_H_

#include "defines.h"

#include "rendering/vulkan/re_vulkan_context.h"
#include "rendering/re_renderer_backend.h"
#include "math/vector.h"

#include <vulkan/vulkan.h>

#include <vector>

class reIvkRenderer : public reIceRendererBackend
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
  // 
  b8 CreateRenderpass();
  b8 CreateDepthImage();
  b8 CreateFrameBuffers();

  b8 CreateSyncObjects();
  b8 CreateCommandBuffers();

  // Platform =====
  // Retrieves all of the extensions the platform requires to render and present with Vulkan
  void GetPlatformExtensions(std::vector<const char*>& _extensions);
  // Creates a vendor-specific surface for display
  b8 CreateSurface();
  vec2U GetPlatformWindowExtents();

  // Helpers =====
  b8 CreateImage(reIvkImage* _image, VkExtent2D _extents, VkFormat _format, VkImageUsageFlags _usage);
  b8 CreateImageView(reIvkImage* _image, VkImageAspectFlags _aspectFlags);
  b8 CreateImageSampler(reIvkImage* _image);

};

#endif // !define ICE_RENDERING_RE_RENDERER_VULKAN_H_
