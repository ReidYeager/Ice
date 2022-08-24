
#ifndef ICE_RENDERING_VULKAN_H_
#define ICE_RENDERING_VULKAN_H_

#include "defines.h"

#include "rendering/renderer.h"
#include "rendering/renderer_defines.h"

#include <vulkan/vulkan.h>

#include <vector>

namespace Ice
{
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

    VkDescriptorSetLayout cameraDescriptorLayout;
    VkDescriptorSetLayout objectDescriptorLayout;
    VkDescriptorSet objectDescriptorSet;

    Ice::IvkRenderpass forward;
    Ice::IvkRenderpass deferred;

    // Commands =====
    std::vector<VkCommandBuffer> commandBuffers;
  };

  class RendererVulkan : public Ice::Renderer
  {
  private:
    Ice::VulkanContext context;
    Ice::Image defaultTexture;

  private:
    //=========================
    // Vulkan Initialization
    //=========================

    // Ensures that all desired layer and extension functionality is present in the created instance
    b8 CreateInstance(const char* _title, u32 _version);
    // Selects the first compatible GPU among those available in the system
    // Fills the selected GPU's information
    b8 ChoosePhysicalDevice();
    // Returns the first queue that satisfies the input flags
    u32 GetIndexForQueue(VkQueueFlags _flags);
    // Finds the first GPU queue that supports presentation
    u32 GetPresentQueue();
    // Tests the GPU for desired functionality
    u32 IsDeviceSuitable(const VkPhysicalDevice& _device);
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
    Ice::vec2U GetWindowExtents();

    //=========================
    // Renderpasses
    //=========================

    b8 CreateDepthImages();
    b8 CreateForwardComponents();
    b8 CreateDeferredComponents();

    //=========================
    // Commands
    //=========================

    VkCommandBuffer BeginSingleTimeCommand(VkCommandPool _pool);
    b8 EndSingleTimeCommand(VkCommandBuffer& _command, VkCommandPool _pool, VkQueue _queue);
    b8 RecordCommandBuffer(u32 _commandIndex, Ice::FrameInformation* _data);

    //=========================
    // Image
    //=========================

    b8 CreateImage(Ice::IvkImage* _image,
                   VkExtent2D _extents,
                   VkFormat _format,
                   VkImageUsageFlags _usage);

    b8 CreateImageView(VkImageView* _view,
                       VkImage _image,
                       VkFormat _format,
                       VkImageAspectFlags _aspectMask);

    b8 CreateImageSampler(Ice::Image* _image);

    void TransitionImageLayout(Ice::Image* _image,
                               b8 _toWritable,
                               VkPipelineStageFlagBits _shaderStage);
    void CopyBufferToImage(Ice::Buffer* _buffer, Ice::Image* _image);

    b8 DestroyImage(Ice::IvkImage* _image);

    //=========================
    // Material
    //=========================

    b8 CreateGlobalDescriptors();

    b8 CreateShaderModule(Ice::Shader* _shader);
    // Attempts to read the shader's descriptor file
    // Shader descriptors are optional so this can not fail
    void LoadShaderDescriptors(Ice::Shader* _shader);
    b8 AssembleMaterialDescriptorBindings(Ice::Material* _material,
                                          std::vector<VkDescriptorSetLayoutBinding>& _bindings);
    b8 CreateDescriptorLayoutAndSet(std::vector<VkDescriptorSetLayoutBinding>* bindings,
                                    VkDescriptorSetLayout* _layout,
                                    VkDescriptorSet* _set);
    b8 CreateDescriptorLayout(std::vector<VkDescriptorSetLayoutBinding>* _bindings,
                              VkDescriptorSetLayout* _layout);
    b8 CreateDescriptorSet(VkDescriptorSetLayout* _layout, VkDescriptorSet* _set);
    // Collects the descriptors for all shaders to create the material's layout/set
    b8 CreateDescriptorLayoutAndSet(Ice::Material* _material);
    void UpdateDescriptorSet(VkDescriptorSet& _set,
                             const std::vector<Ice::ShaderInputElement>& _bindings);
    b8 CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& _setLayouts,
                            VkPipelineLayout* _pipelineLayout);
    b8 CreatePipeline(Ice::Material* _material);

    //=========================
    // Buffer
    //=========================

    b8 FlushBufferQueue();
    u64 PadBufferSize(u64 _inSize, Ice::BufferMemoryUsageFlags _usage);

  public:
    b8 Init(Ice::RendererSettingsCore _settings,
            const char* _title = "Ice_Vk_Application",
            u32 _version = 0);
    b8 RenderFrame(Ice::FrameInformation* _data);
    b8 Shutdown();

    b8 CreateShader(Ice::ShaderSettings _settings, Ice::Shader* _shader);
    void DestroyShader(Ice::Shader& _shader);
    b8 ReloadShader(Ice::Shader* _shader);
    b8 CreateMaterial(Ice::Material* _material);
    void DestroyMaterial(Ice::Material& _material);
    b8 RecreateMaterial(Ice::Material* _material);

    b8 SetMaterialInput(Ice::Material* _material, u32 _bindIndex, Ice::Image* _image);

    b8 CreateTexture(Ice::Image* _image, void* _data);
    void DestroyImage(Ice::Image* _image);

    b8 CreateBufferMemory(Ice::Buffer* _outBuffer,
                          u64 _elementSize,
                          u32 _elementCount,
                          Ice::BufferMemoryUsageFlags _usage);
    void DestroyBufferMemory(Ice::Buffer* _buffer);
    // Pushes data to the GPU immediately
    b8 PushDataToBuffer(void* _data, const Ice::BufferSegment _segmentInfo);
    //// Adds data to a set to be pushed to the GPU just before rendering
    //// Assumes _data will remain valid through the frame
    //// (Left here because I may want to add this, but will otherwise forget about it)
    //b8 QueueDataToBuffer(void* _data,
    //                     const Ice::Buffer* _buffer,
    //                     const Ice::BufferSegment _segmentInfo);

    b8 InitializeRenderComponent(Ice::RenderComponent* _component,
                                 Ice::BufferSegment* _TransformBuffer);
    b8 UpdateRenderComponent(Ice::RenderComponent* _component,
                             Ice::BufferSegment* _TransformBuffer);
    b8 InitializeCamera(Ice::CameraComponent* _camera,
                        Ice::BufferSegment _transformSegment,
                        Ice::CameraSettings _settings);
  };

} // namespace Ice

#endif // !define ICE_RENDERING_VULKAN_H_
