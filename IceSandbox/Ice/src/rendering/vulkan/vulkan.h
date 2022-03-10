
#ifndef ICE_RENDERING_VULKAN_H_
#define ICE_RENDERING_VULKAN_H_

#include "defines.h"

#include "rendering/renderer.h"
#include "rendering/vulkan/vulkan_context.h"

#include <vulkan/vulkan.h>

#include <vector>

namespace Ice
{

  class RendererVulkan : public Ice::Renderer
  {
  private:
    Ice::VulkanContext context;

  private:
    //=========================
    // Vulkan Initialization
    //=========================

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
    // Creates the fences and semaphores for CPU/GPU synchronization
    b8 CreateSyncObjects();
    // Creates a command buffer for each frame
    b8 CreateCommandBuffers();

    //=========================
    // Platform
    //=========================

    // Retrieves all of the extensions the platform requires to render and present with Vulkan
    void GetRequiredPlatformExtensions(std::vector<const char*>& _extensions);
    // Creates a vendor-specific surface for display
    b8 CreateSurface();
    // Gets the extents of the current window
    vec2U GetWindowExtents();

    //=========================
    // Commands
    //=========================


    //=========================
    // Image
    //=========================

    b8 CreateImageView(VkImageView* _view,
                       VkImage _image,
                       VkFormat _format,
                       VkImageAspectFlags _aspectMask);

  public:
    b8 Init(Ice::RendererSettings _settings);
    b8 Shutdown();
  };

}
#endif // !define ICE_RENDERING_VULKAN_H_
