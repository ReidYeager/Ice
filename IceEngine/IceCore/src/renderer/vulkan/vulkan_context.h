
#ifndef ICE_RENDERER_VULKAN_VULKAN_CONTEXT_H_
#define ICE_RENDERER_VULKAN_VULKAN_CONTEXT_H_

#include "defines.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <set>
#include <string>

inline const char* VulkanResultToString(VkResult _result)
{
  #define RTS(x) case x: return #x;

  switch (_result)
  {
    RTS(VK_SUCCESS);
    RTS(VK_NOT_READY);
    RTS(VK_TIMEOUT);
    RTS(VK_EVENT_SET);
    RTS(VK_EVENT_RESET);
    RTS(VK_INCOMPLETE);
    RTS(VK_ERROR_OUT_OF_HOST_MEMORY);
    RTS(VK_ERROR_OUT_OF_DEVICE_MEMORY);
    RTS(VK_ERROR_INITIALIZATION_FAILED);
    RTS(VK_ERROR_DEVICE_LOST);
    RTS(VK_ERROR_MEMORY_MAP_FAILED);
    RTS(VK_ERROR_LAYER_NOT_PRESENT);
    RTS(VK_ERROR_EXTENSION_NOT_PRESENT);
    RTS(VK_ERROR_FEATURE_NOT_PRESENT);
    RTS(VK_ERROR_INCOMPATIBLE_DRIVER);
    RTS(VK_ERROR_TOO_MANY_OBJECTS);
    RTS(VK_ERROR_FORMAT_NOT_SUPPORTED);
    RTS(VK_ERROR_FRAGMENTED_POOL);
    RTS(VK_ERROR_UNKNOWN);
    RTS(VK_ERROR_OUT_OF_POOL_MEMORY);
    RTS(VK_ERROR_INVALID_EXTERNAL_HANDLE);
    RTS(VK_ERROR_FRAGMENTATION);
    RTS(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS);
    RTS(VK_ERROR_SURFACE_LOST_KHR);
    RTS(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
    RTS(VK_SUBOPTIMAL_KHR);
    RTS(VK_ERROR_OUT_OF_DATE_KHR);
    RTS(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);
    RTS(VK_ERROR_VALIDATION_FAILED_EXT);
    RTS(VK_ERROR_INVALID_SHADER_NV);
    RTS(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT);
    RTS(VK_ERROR_NOT_PERMITTED_EXT);
    RTS(VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT);
    RTS(VK_THREAD_IDLE_KHR);
    RTS(VK_THREAD_DONE_KHR);
    RTS(VK_OPERATION_DEFERRED_KHR);
    RTS(VK_OPERATION_NOT_DEFERRED_KHR);
    RTS(VK_PIPELINE_COMPILE_REQUIRED_EXT);
    RTS(VK_RESULT_MAX_ENUM);
  default: return "Invalid vkResult";
  }

  #undef ETS
}

#define IVK_ASSERT(vresult, errorMsg, ...)      \
{                                               \
  VkResult vkAssertResult = vresult;         \
  if (vkAssertResult != VK_SUCCESS)             \
  {                                             \
    IcePrint(errorMsg, __VA_ARGS__);        \
    throw VulkanResultToString(vkAssertResult); \
  }                                             \
}

struct iceImage_t
{
  VkImage image;
  VkDeviceMemory memory;
  VkFormat format;
  VkImageView view;
  VkSampler sampler;
};

struct IcePhysicalDevice
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

struct RenderSynchronization
{
  // Synchronization
  #define MAX_FLIGHT_IMAGE_COUNT 3
  std::vector<VkFence> imageIsInFlightFences;
  std::vector<VkFence> flightFences;
  std::vector<VkSemaphore> renderCompleteSemaphores;
  std::vector<VkSemaphore> imageAvailableSemaphores;
  u32 currentFrame = 0;
};

struct IceRenderContext
{
  VkAllocationCallbacks* allocator;

  VkInstance instance;
  VkSurfaceKHR surface;

  IcePhysicalDevice gpu;
  VkDevice device;

  u32 graphicsIdx;
  u32 presentIdx;
  u32 transferIdx;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
  VkQueue transferQueue;

  VkCommandPool graphicsCommandPool;
  VkCommandPool transientCommandPool;

  VkExtent2D renderExtent;
  VkRenderPass renderPass;

  RenderSynchronization syncObjects;

  VkSwapchainKHR swapchain;
  VkFormat swapchainFormat;

  std::vector<VkImage> swapchainImages;
  std::vector<VkImageView> swapchainImageViews;

  iceImage_t* depthImage;
  std::vector<VkFramebuffer> frameBuffers;

  std::vector<VkCommandBuffer> commandBuffers;
  VkDescriptorPool descriptorPool;

  // Handles all setup for recording a commandBuffer to be executed once
  VkCommandBuffer BeginSingleTimeCommand(VkCommandPool& _pool)
  {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = _pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer singleCommand;
    IVK_ASSERT(vkAllocateCommandBuffers(device, &allocInfo, &singleCommand),
      "Failed to create transient command buffer");

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(singleCommand, &beginInfo);

    return singleCommand;
  }

  // Handles the execution and destruction of a commandBuffer
  void EndSingleTimeCommand(VkCommandBuffer _command, VkCommandPool& _pool, VkQueue& _queue)
  {
    vkEndCommandBuffer(_command);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_command;

    vkQueueSubmit(_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_queue);

    vkFreeCommandBuffers(device, _pool, 1, &_command);
  }
};

#endif // !ICE_RENDERER_VULKAN_VULKAN_CONTEXT_H_
