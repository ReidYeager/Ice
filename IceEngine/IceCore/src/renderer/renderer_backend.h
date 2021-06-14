
#ifndef RENDERER_RENDERER_BACKEND_H
#define RENDERER_RENDERER_BACKEND_H 1

#include "defines.h"
#include "renderer/shader_program.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <glm/glm.hpp>

struct iceImage_t
{
  VkImage image;
  VkDeviceMemory memory;
  VkFormat format;
  VkImageView view;
  //VkSampler sampler;
};

//struct iceBuffer_t
//{
//  VkBuffer buffer;
//  VkDeviceMemory memory;
//  //u32 size;
//};

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

  // TODO : Delete
  struct mvpMatrices
  {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
  } mvp;
  VkBuffer mvpBuffer;
  VkDeviceMemory mvpBufferMemory;

//=================================================================================================
// VARIABLES
//=================================================================================================
private:
  IceVulkanState vState {};

  std::vector<const char*> deviceLayers =
      { "VK_LAYER_KHRONOS_validation" };
  std::vector<const char*> instanceExtensions = 
      { VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_win32_surface" };
  std::vector<const char*> deviceExtensions =
      { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
  VkInstance instance;
  VkSurfaceKHR surface;

  VkSwapchainKHR swapchain;
  VkFormat swapchainFormat;
  std::vector<VkImage> swapchainImages;
  std::vector<VkImageView> swapchainImageViews;
  iceImage_t* depthImage = nullptr;
  std::vector<VkFramebuffer> frameBuffers;

  VkDescriptorPool descriptorPool;

  std::vector<iceImage_t*> iceImages;

  // Synchronization
  #define MAX_FLIGHT_IMAGE_COUNT 3
  std::vector<VkFence> imageIsInFlightFences;
  std::vector<VkFence> flightFences;
  std::vector<VkSemaphore> renderCompleteSemaphores;
  std::vector<VkSemaphore> imageAvailableSemaphores;
  u32 currentFrame = 0;

  std::vector<VkCommandBuffer> commandBuffers;

  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorSet descriptorSet;
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;

//=================================================================================================
// FUNCTIONS
//=================================================================================================
public:
  // Initializes the fundamentals required to create renderer components
  RendererBackend();
  // Destroys all components used for rendering & presentation
  ~RendererBackend();
  // Creates components required to render images
  void CreateComponents();
  // Destroys rendering components
  void DestroyComponents();
  // Destroys and recreates rendering components
  void RecreateComponents();

  // NOTE : Temporary?
  void RenderFrame();

  // Creates a new iceShader
  iceShader_t CreateShader(const char* _name, IceShaderStageFlags _stage);
  // TODO : I don't like this. Find a better way to destroy shaders
  void DestroyShaderModule(VkShaderModule& _shader);

  // Creates a buffer on the GPU and fills it with data
  void CreateAndFillBuffer(VkBuffer& _buffer, VkDeviceMemory& _mem, const void* _data,
                           VkDeviceSize _size, VkBufferUsageFlags _usage);
  void CreateBuffer(VkBuffer& _buffer, VkDeviceMemory& _mem, VkDeviceSize _size,
                    VkBufferUsageFlags _usage, VkMemoryPropertyFlags _memProperties);
  void FillBuffer(VkDeviceMemory& _mem, const void* _data, VkDeviceSize _size);
  void CopyBuffer(VkBuffer _src, VkBuffer _dst, VkDeviceSize _size);
  void DestroyBuffer(VkBuffer _buffer, VkDeviceMemory _memory);

  void RecordCommandBuffers();

private:
  // Calls CreateComponents
  // Creates components that live through recreation
  void InitializeComponents();

  void CreateInstance();
  void CreateDevice();
  void ChoosePhysicalDevice(VkPhysicalDevice& _selectedDevice, u32& _graphicsIndex,
                          u32& _presentIndex, u32& _transferIndex);
  void CreateCommandPool(VkCommandPool& _pool, u32 _queueIndex,
                         VkCommandPoolCreateFlags _flags = 0);

  // Render components
  void CreateSwapchain();
  void CreateRenderpass();
  void CreateDepthImage();
  void CreateFramebuffers();

  void CreateDescriptorPool();
  void CreateSyncObjects();
  void CreateCommandBuffers();

  // TODO : Move to shader programs?
  void CreateDescriptorSetLayout();
  void CreatePipelineLayout();
  void CreatePipeline();
  void CreateDescriptorSet();

  // ===== Helpers =====
  // Returns the first instance of a queue with the input flags
  u32 GetQueueIndex(std::vector<VkQueueFamilyProperties>& _queues, VkQueueFlags _flags);
  // Returns the first instance of a presentation queue
  u32 GetPresentIndex(
      const VkPhysicalDevice* _device, u32 _queuePropertyCount, u32 _graphicsIndex);
  VkFormat FindDepthFormat();
  VkFormat FindSupportedFormat(const std::vector<VkFormat>& _formats,
                               VkImageTiling _tiling, VkFormatFeatureFlags _features);
  u32 FindMemoryType(u32 _mask, VkMemoryPropertyFlags _flags);

  // TODO : Move to an image manager?
  u32 CreateImage(u32 _width, u32 _height, VkFormat _format, VkImageTiling _tiling,
    VkImageUsageFlags _usage, VkMemoryPropertyFlags _memProps);
  VkImageView CreateImageView(
    const VkFormat _format, VkImageAspectFlags _aspect, const VkImage& _image);
  

};

#endif // !RENDERER_RENDER_BACKEND_H
