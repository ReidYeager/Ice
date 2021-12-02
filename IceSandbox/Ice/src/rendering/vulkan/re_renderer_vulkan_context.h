
#ifndef ICE_RENDERING_VULKAN_RE_RENDERER_VULKAN_CONTEXT_H_
#define ICE_RENDERING_VULKAN_RE_RENDERER_VULKAN_CONTEXT_H_

#include "defines.h"
#include "asserts.h"

#include "rendering\vulkan\re_vulkan_assert.h"

#include <vulkan/vulkan.h>

#include <vector>

struct reIvkGpu
{
  VkPhysicalDevice                 device;
  VkPhysicalDeviceProperties       properties;
  VkPhysicalDeviceFeatures         features;
  VkPhysicalDeviceMemoryProperties memoryProperties;
  VkSurfaceCapabilitiesKHR         surfaceCapabilities;

  std::vector<VkSurfaceFormatKHR>      surfaceFormats;
  std::vector<VkPresentModeKHR>        presentModes;
  std::vector<VkQueueFamilyProperties> queueFamilyProperties;

  u32 graphicsQueueIndex;
  u32 presentQueueIndex;
  u32 transferQueueIndex;
};

struct reIvkContext
{
  VkAllocationCallbacks* alloc = nullptr;

  VkInstance   instance = VK_NULL_HANDLE;
  VkSurfaceKHR surface  = VK_NULL_HANDLE;
  VkDevice     device   = VK_NULL_HANDLE;
  reIvkGpu     gpu;

  VkQueue graphicsQueue;
  VkQueue presentQueue;
  VkQueue transferQueue;

};

#endif // !define ICE_RENDERING_VULKAN_RE_RENDERER_VULKAN_CONTEXT_H_
