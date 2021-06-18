
#ifndef RENDERER_BACKEND_CONTEXT_H
#define RENDERER_BACKEND_CONTEXT_H 1

#include "defines.h"

#ifdef ICE_VULKAN
#include <vulkan/vulkan.h>
#include <vector>
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

struct IceRenderContext
{
  VkAllocationCallbacks* allocator;

  IcePhysicalDeviceInformation gpu;
  VkDevice device;

  uint32_t graphicsIdx;
  uint32_t presentIdx;
  uint32_t transferIdx;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
  VkQueue transferQueue;

  VkCommandPool graphicsCommandPool;
  VkCommandPool transientCommandPool;

  VkExtent2D renderExtent;
  VkRenderPass renderPass;
};

extern IceRenderContext rContext;

#endif

#endif // !RENDERER_BACKEND_CONTEXT_H
