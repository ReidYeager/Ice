
#ifndef ICE_RENDERING_VULKAN_RE_VULKAN_CONTEXT_H_
#define ICE_RENDERING_VULKAN_RE_VULKAN_CONTEXT_H_

#include "defines.h"
#include "asserts.h"

#include "math/vector.h"

#include <vulkan/vulkan.h>

#include <vector>

// =======================
// Buffer
// =======================

struct IvkBuffer
{
  //IvkBuffer* parent; // Used in sub-allocating buffers
  VkBuffer buffer = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  u64 size = 0; // Not Padded
  u64 offset = 0;
};

// =======================
// Image
// =======================

struct reIvkImage
{
  vec2U extents;
  VkImage image = VK_NULL_HANDLE;
  VkImageView view = VK_NULL_HANDLE;
  VkSampler sampler = VK_NULL_HANDLE;

  VkFormat format;
  VkImageLayout layout;

  VkDeviceMemory memory;
};

// =======================
// Material
// =======================

enum IceShaderStageFlagBits
{
  Ice_Shader_Invalid = 0,
  Ice_Shader_Vertex,
  Ice_Shader_Fragment
};
typedef IceFlag IceShaderStageFlag;

struct IceShaderInfo
{
  const char* directory;
  IceShaderStageFlag stages;
};

struct IvkShader
{
  VkShaderModule module;
  IceShaderInfo info;
};

struct IvkMaterial
{
  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorSet descriptorSet;

  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;
  IvkShader vertexModule;
  IvkShader fragmentModule;
};

// =======================
// Context
// =======================

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
  u32 transientQueueIndex;
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
  VkQueue transientQueue;

  VkDescriptorPool descriptorPool;

  VkCommandPool graphicsCommandPool;
  VkCommandPool transientCommandPool; // Used only for one-time commands
  std::vector<VkCommandBuffer> commandsBuffers;

  VkSwapchainKHR swapchain;
  std::vector<VkImage> swapchainImages;
  std::vector<VkImageView> swapchainImageViews;
  VkFormat swapchainFormat;
  VkExtent2D swapchainExtent;

  reIvkImage depthImage;

  VkRenderPass renderpass;
  std::vector<VkFramebuffer> frameBuffers;

  std::vector<VkFence> flightSlotAvailableFences;
  std::vector<VkSemaphore> renderCompleteSemaphores;
  std::vector<VkSemaphore> imageAvailableSemaphores;
  u32 currentFlightIndex = 0;
  #define RE_MAX_FLIGHT_IMAGE_COUNT 3
};

// =======================
// Vulkan assert
// =======================

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

#define IVK_ASSERT(function, errorMsg, ...)                                      \
{                                                                                \
  VkResult result = function;                                                    \
  if (result != VK_SUCCESS)                                                      \
  {                                                                              \
    char msg[2048];                                                              \
    sprintf(msg, errorMsg, __VA_ARGS__);                                         \
    ICE_ASSERT_MSG(result == VK_SUCCESS,                                         \
                   "%s\nVulkan result : %s", msg, VulkanResultToString(result)); \
    return false;                                                                \
  }                                                                              \
}

#endif // !define ICE_RENDERING_VULKAN_RE_RENDERER_VULKAN_CONTEXT_H_
