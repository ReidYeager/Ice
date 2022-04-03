
#ifndef ICE_RENDERING_VULKAN_H_
#define ICE_RENDERING_VULKAN_H_

#include "defines.h"

#include "rendering/renderer.h"
#include "rendering/vulkan/vulkan_context.h"

#include <vulkan/vulkan.h>

#include <vector>

namespace Ice
{
  class RendererVulkan : public Ice::Renderer
  {
  private:
    Ice::VulkanContext context;

  private:
    //=========================
    // Vulkan Initialization
    //=========================

    // Ensures that all desired layer and extension functionality is present in the created instance
    b8 CreateInstance();
    // Selects the first compatible GPU among those available in the system
    // Fills the selected GPU's information
    b8 ChoosePhysicalDevice();
    // Returns the first queue that satisfies the input flags
    u32 GetIndexForQueue(VkQueueFlags _flags);
    // Finds the first GPU queue that supports presentation
    u32 GetPresentQueue();
    // Tests the GPU for desired functionality
    b8 IsDeviceSuitable(const VkPhysicalDevice& _device);
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
    vec2U GetWindowExtents();

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

    b8 DestroyImage(Ice::IvkImage* _image);

    //=========================
    // Material
    //=========================

    b8 CreateShaderModule(Ice::Shader* _shader);
    // Attempts to read the shader's descriptor file
    // Shader descriptors are optional so this can not fail
    void LoadShaderDescriptors(Ice::Shader* _shader);
    b8 CreateDescriptorLayoutAndSet(const Ice::MaterialSettings& _settings,
                                    Ice::Material* _material);
    b8 CreatePipelineLayout(const Ice::MaterialSettings& _settings, Ice::Material* _material);
    b8 CreatePipeline(Ice::MaterialSettings _settings, Ice::Material* _material);

    //=========================
    // Buffer
    //=========================

    b8 FlushBufferQueue();

  public:
    b8 Init(Ice::RendererSettings _settings);
    b8 RenderFrame(Ice::FrameInformation* _data);
    b8 Shutdown();

    b8 CreateShader(Ice::Shader* _shader);
    void DestroyShader(Ice::Shader& _shader);
    b8 CreateMaterial(Ice::Material* _material, Ice::MaterialSettings _settings);
    void DestroyMaterial(Ice::Material& _material);

    b8 CreateBufferMemory(Ice::Buffer* _outBuffer, u64 _size, Ice::BufferMemoryUsageFlags _usage);
    void DestroyBufferMemory(Ice::Buffer* _buffer);
    // Pushes data to the GPU immediately
    b8 PushDataToBuffer(void* _data,
                        const Ice::Buffer* _buffer,
                        const Ice::BufferSegment* _segmentInfo = nullptr);
    // Adds data to a set to be pushed to the GPU just before rendering
    // Assumes _data will remain valid through the frame
    b8 QueueDataToBuffer(void* _data,
                         const Ice::Buffer* _buffer,
                         const Ice::BufferSegment* _segmentInfo = nullptr);
  };

}
#endif // !define ICE_RENDERING_VULKAN_H_
