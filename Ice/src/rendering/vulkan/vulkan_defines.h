
#ifndef ICE_RENDERING_VULKAN_CONTEXT_H_
#define ICE_RENDERING_VULKAN_CONTEXT_H_

#include "defines.h"

#include "math/vector.hpp"

#include <vulkan/vulkan.h>

#include <vector>

namespace Ice {

//=========================
// Image
//=========================

struct IvkImage
{
  VkImage image;
  VkImageView view;
  VkSampler sampler;

  VkFormat format;
  VkImageLayout layout;

  VkDeviceMemory memory;
};

//=========================
// Buffer
//=========================

struct IvkBuffer
{
  VkBuffer buffer;
  VkDeviceMemory memory;
};

//=========================
// Material
//=========================

struct IvkShader
{
  VkShaderModule module;
};

struct IvkMaterial
{
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;
  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorSet descriptorSet;
};

//=========================
// Context
//=========================

struct IvkRenderpass
{
  VkRenderPass renderpass;
  std::vector<VkFramebuffer> framebuffers;
};

struct Gpu
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
  u32 transientQueueIndex;
};

struct IvkObjectData
{
  VkDescriptorSet descriptorSet; // Per-object input data
};

} // namespace Ice

//=======================
// Assert
//=======================

inline const char* VulkanResultToString(VkResult _result)
{
#define RTS(x) case x: return #x
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

#define IVK_ASSERT(function, errorMsg, ...)                                            \
{                                                                                      \
  VkResult result = function;                                                          \
  if (result != VK_SUCCESS)                                                            \
  {                                                                                    \
    char msg[2048];                                                                    \
    sprintf(msg, errorMsg, __VA_ARGS__);                                               \
    ICE_ASSERT_MSG(result == VK_SUCCESS,                                               \
                   ">> %s\n>> Vulkan result : %s", msg, VulkanResultToString(result)); \
  }                                                                                    \
}

#endif // !ICE_RENDERING_VULKAN_CONTEXT_H_
