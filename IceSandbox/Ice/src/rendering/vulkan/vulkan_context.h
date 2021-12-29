
#ifndef ICE_RENDERING_VULKAN_VULKAN_CONTEXT_H_
#define ICE_RENDERING_VULKAN_VULKAN_CONTEXT_H_

#include "defines.h"
#include "asserts.h"

#include "math/vector.h"

#include <vulkan/vulkan.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_vulkan.h"
#include "imgui/imgui_impl_win32.h"

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
// Descriptor
// =======================

struct IvkDescriptor
{
  VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL;
};

struct IvkDescriptorBinding
{
  VkDescriptorType type;
  reIvkImage* image = nullptr;
  IvkBuffer* buffer = nullptr;
};

//=========================
// Renderpass
//=========================

struct IvkAttachmentDescRef
{
  VkAttachmentDescription description;
  VkAttachmentReference reference;
};

struct IvkAttachmentSettings
{
  VkFormat imageFormat;
  VkAttachmentLoadOp loadOperation = VK_ATTACHMENT_LOAD_OP_CLEAR;
  VkAttachmentStoreOp storeOperation = VK_ATTACHMENT_STORE_OP_STORE;
  VkImageLayout finalLayout;
  VkImageLayout referenceLayout;
};

struct IvkSubpassSettings
{
  std::vector<u32> colorIndices;
  u32 depthIndex = ~(0u); // ~0 is an invalid index
  VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
};

struct IvkSubpassDependencies
{
  u32 srcIndex;
  u32 dstIndex;
};

//=========================
// Shader
//=========================

enum IceShaderStageFlagBits
{
  Ice_Shader_Invalid = 0,
  Ice_Shader_Vertex = 1,
  Ice_Shader_Fragment = 2,
  Ice_shader_VertFrag = Ice_Shader_Vertex | Ice_Shader_Fragment
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

//=========================
// Material
//=========================

struct IvkMaterial
{
  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorSet descriptorSet;

  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;
  VkPipeline shadowPipeline;
  IvkShader vertexModule;
  IvkShader fragmentModule;
};

// =======================
// TMP -- Lights
// =======================

struct IvkLights
{
  vec4 directionalDirection;
  vec4 directionalColor;
};

struct IvkShadow
{
  VkFramebuffer framebuffer;
  reIvkImage image;
  VkRenderPass renderpass = VK_NULL_HANDLE;
  VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
  IvkBuffer lightMatrixBuffer;

  glm::mat4 viewProjMatrix;
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
  VkSurfaceFormatKHR surfaceFormat;
  VkDevice     device   = VK_NULL_HANDLE;
  reIvkGpu     gpu;

  VkQueue graphicsQueue;
  VkQueue presentQueue;
  VkQueue transientQueue;

  VkDescriptorPool descriptorPool;
  VkDescriptorPool imguiPool;
  ImGui_ImplVulkanH_Window imguiWindow;

  VkCommandPool graphicsCommandPool;
  VkCommandPool transientCommandPool; // Used only for one-time commands
  std::vector<VkCommandBuffer> commandsBuffers;

  VkSwapchainKHR swapchain;
  VkPresentModeKHR presentMode;
  std::vector<VkImage> swapchainImages;
  std::vector<VkImageView> swapchainImageViews;
  VkFormat swapchainFormat;
  VkExtent2D swapchainExtent;

  reIvkImage depthImage;

  VkRenderPass mainRenderpass;
  std::vector<VkFramebuffer> frameBuffers;

  std::vector<VkFence> flightSlotAvailableFences;
  std::vector<VkSemaphore> renderCompleteSemaphores;
  std::vector<VkSemaphore> imageAvailableSemaphores;
  u32 currentFlightIndex = 0;
  #define RE_MAX_FLIGHT_IMAGE_COUNT 3

  VkDescriptorSetLayout objectDescriptorSetLayout;

  VkDescriptorSetLayout globalDescriptorSetLayout;
  VkDescriptorSet globalDescritorSet;
  VkPipelineLayout globalPipelinelayout;
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
