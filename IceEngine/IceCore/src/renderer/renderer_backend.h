
#ifndef RENDERER_RENDERER_BACKEND_H
#define RENDERER_RENDERER_BACKEND_H 1
// TODO : Define API agnostic calls

#include "defines.h"

#include <vulkan/vulkan.h>
#include <vector>

class RendererBackend
{
private:
  struct IcePhysicalDeviceInformation
  {
    VkPhysicalDevice device;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceMemoryProperties memProperties;
    VkPhysicalDeviceFeatures features;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    std::vector<VkPresentModeKHR> presentModes;
    std::vector<VkQueueFamilyProperties> queueFamilyProperties;
    std::vector<VkExtensionProperties> extensionProperties;
  };

  struct IceVulkanState
  {
    IcePhysicalDeviceInformation gpu;
    VkDevice device;

    uint32_t graphicsIdx;
    uint32_t presentIdx;
    uint32_t transferIdx;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue transferQueue;

    VkCommandPool graphicsCommandPool;

    VkExtent2D renderExtent;
    VkRenderPass renderPass;
  };

//=================================================================================================
// VARIABLES
//=================================================================================================
private:
  IceVulkanState vState {};

  std::vector<const char*> deviceLayers = { "VK_LAYER_KHRONOS_validation" };
  std::vector<const char*> instanceExtensions = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
                                                  VK_KHR_SURFACE_EXTENSION_NAME,
                                                  "VK_KHR_win32_surface" };
  std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
  VkInstance instance;
  VkSurfaceKHR surface;

//=================================================================================================
// FUNCTIONS
//=================================================================================================
public:
  // Initializes the fundamentals required to create renderer components
  RendererBackend();
  // Destroys the fundamentals required to create renderer components
  ~RendererBackend();
  // Creates the basic set of components required to render images
  i8 CreateComponents();
  // Destroys all components
  i8 DestroyComponents();

private:
  i8 CreateInstance();
  i8 CreateDevice();
  i8 ChoosePhysicalDevice(VkPhysicalDevice& _selectedDevice, u32& _graphicsIndex,
                          u32& _presentIndex, u32& _transferIndex);

  // Returns the first instance of a queue with the input flags
  u32 GetQueueIndex(std::vector<VkQueueFamilyProperties>& _queues, VkQueueFlags _flags);
  // Returns the first instance of a presentation queue
  u32 GetPresentIndex(
      const VkPhysicalDevice* _device, u32 _queuePropertyCount, u32 _graphicsIndex);

};

#endif // !RENDERER_RENDER_BACKEND_H
