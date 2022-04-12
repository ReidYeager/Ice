
#ifndef ICE_RENDERING_VULKAN_CONTEXT_H_
#define ICE_RENDERING_VULKAN_CONTEXT_H_

#include "defines.h"
#include "logger.h"

#include "rendering/renderer.h"
#include "math/vector.h"

#include <vulkan/vulkan.h>

#include <vector>

namespace Ice {

  //=========================
  // Image
  //=========================

  struct IvkImage
  {
    vec2U extents;
    VkImage image = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;

    VkFormat format;
    VkImageLayout layout;

    VkDeviceMemory memory;
  };

  //=========================
  // Mesh
  //=========================

  struct IvkVertex
  {
    Ice::Vertex vertex;

    static VkVertexInputBindingDescription GetBindingDescription()
    {
      VkVertexInputBindingDescription desc = {};
      desc.stride = sizeof(Ice::Vertex);
      desc.binding = 0;
      desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      return desc;
    }

    static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions()
    {
      std::vector<VkVertexInputAttributeDescription> attribs(3);
      // Position
      attribs[0].binding = 0;
      attribs[0].location = 0;
      attribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribs[0].offset = offsetof(Ice::Vertex, position);
      // UV
      attribs[2].binding = 0;
      attribs[2].location = 1;
      attribs[2].format = VK_FORMAT_R32G32_SFLOAT;
      attribs[2].offset = offsetof(Ice::Vertex, uv);
      // normal
      attribs[1].binding = 0;
      attribs[1].location = 2;
      attribs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribs[1].offset = offsetof(Ice::Vertex, normal);

      return attribs;
    }
  };

  //=========================
  // Context
  //=========================

  struct Renderpass
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

  struct VulkanContext
  {
    VkAllocationCallbacks* alloc = nullptr;

    VkInstance instance;
    VkSurfaceKHR surface;
    VkSurfaceFormatKHR surfaceFormat;
    VkDevice device;
    Gpu gpu;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue transientQueue;

    VkDescriptorPool descriptorPool;
    VkCommandPool graphicsCommandPool;
    VkCommandPool transientCommandPool;

    // Swapchain =====
    VkPresentModeKHR presentMode;
    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    VkFormat swapchainFormat;
    VkExtent2D swapchainExtent;

    // Depth =====
    std::vector<IvkImage> depthImages;

    // Synchronization =====
    std::vector<VkFence> flightSlotAvailableFences;
    std::vector<VkSemaphore> renderCompleteSemaphores;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    u32 currentFlightIndex = 0;
    #define ICE_MAX_FLIGHT_IMAGE_COUNT 3

    // Renderpasses =====
    VkDescriptorSet globalDescriptorSet;
    VkDescriptorSetLayout globalDescriptorLayout;
    VkPipelineLayout globalPipelineLayout;
    Ice::Buffer globalDescriptorBuffer;

    VkDescriptorSetLayout objectDescriptorLayout;

    Ice::Renderpass forward;
    Ice::Renderpass deferred;

    // Commands =====
    std::vector<VkCommandBuffer> commandBuffers;
  };

}

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
    return false;                                                                      \
  }                                                                                    \
}

#endif // !ICE_RENDERING_VULKAN_CONTEXT_H_
