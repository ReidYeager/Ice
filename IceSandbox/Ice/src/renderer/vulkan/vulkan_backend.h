
#ifndef ICE_RENDERER_VULKAN_VULKAN_BACKEND_H_
#define ICE_RENDERER_VULKAN_VULKAN_BACKEND_H_

#include "asserts.h"
#include "defines.h"

#include "renderer/vulkan/vulkan_context.h"
#include "renderer/vulkan/vulkan_material.h"
#include "renderer/image.h"
#include "renderer/renderer_backend.h"

class VulkanBackend : public RendererBackend
{
//=================================================================================================
// VARIABLES
//=================================================================================================
private:
  // Houses all the generic vulkan components required for rendering
  IceRenderContext* rContext;

  std::vector<iceImage_t*> iceImages;
  std::vector<iceTexture_t*> iceTextures;

  // Used in stead of a resize event to prevent resizing during rendering
  bool shouldResize = false;

//=================================================================================================
// FUNCTIONS
//=================================================================================================
public:
  // Initializes the generic components required to create materials and render an image
  void Initialize() override;
  // Destroys all generic components used for rendering & presentation
  void Shutdown() override;

  // Queues an image to be rendered to and presents it to the screen
  void RenderFrame(IceRenderPacket* _packet);
  // Records what to render and how to do so
  void RecordCommandBuffers(IceRenderPacket* _packet, u32 _commandIndex);
  // Sets the shouldResize flag
  void Resize(u32 _width = 0, u32 _height = 0) override;

// DELETE
//=================================================================================================
  // Required for command buffer recording
  std::vector<IceMaterial> materials;
  void AddMaterial(IceMaterial _material) override { materials.push_back(_material); }

private:
//=================================================================================================
// Rendering component management
//=================================================================================================

  // Calls CreateComponents
  // Creates components that live through recreation
  void InitializeComponents();
  // Creates components required to render images
  void CreateFragileComponents();
  // Destroys rendering components
  void DestroyFragileComponents();
  // Destroys and recreates rendering components
  void RecreateFragileComponents();

//=================================================================================================
// Static rendering components
//=================================================================================================

  // Creates a vulkan instance
  void InitializeAPI();
  // Chooses a physical device and creates a logical device
  void CreateLogicalDevice();
  // Creates a generic descriptor pool
  // Needs to be reworked
  void CreateDescriptorPool();
  // Creates the fences and sempahores to sync the CPU and GPU
  void CreateSyncObjects();
  // Creates a command pool for the given queue
  void CreateCommandPool(VkCommandPool& _pool,
                         u32 _queueIndex,
                         VkCommandPoolCreateFlags _flags = 0);

  // Retrieves any extensions required to render or present
  void GetRequiredPlatformExtensions(std::vector<const char*>& _extensions);
  // Creates a surface to present to
  void CreateSurface();
  // Gets the pixel sizes of the surface
  VkExtent2D GetWindowExtent();

//=================================================================================================
// Rendering components
//=================================================================================================

  // Each creates its namesake
  // All are fragile components that must be recreated after window resizing

  // Creates a swapchain, its images, and their image views
  void CreateSwapchain();
  // Creates a generic renderpass
  void CreateRenderpass();
  // Creates a depth image and its image view
  void CreateDepthImage();
  void CreateFramebuffers();
  void CreateCommandBuffers();

//=================================================================================================
// GPU
//=================================================================================================

  // Chooses the first GPU to match some arbitrary criteria
  void ChoosePhysicalDevice();
  // Stores the properties and capabilities of the GPU
  void FillPhysicalDeviceInformation();
  // Returns the first instance of a queue with the input flags
  u32 GetQueueIndex(std::vector<VkQueueFamilyProperties>& _queues, VkQueueFlags _flags);
  // Returns the first instance of a presentation queue
  u32 GetPresentIndex(const VkPhysicalDevice* _device,
                      u32 _queuePropertyCount,
                      u32 _graphicsIndex);

//=================================================================================================
// DESCRIPTORS
//=================================================================================================

  void CreateGlobalDescriptorSet();

//=================================================================================================
// NON-API HELPERS
//=================================================================================================

public:
  // Loads the given mesh, creates and fills its vertex and index buffers
  mesh_t CreateMesh(const char* _directory) override;
  iceImage_t* GetImage(u32 _index) override { return iceImages[_index]; }
  // Retrieves or loads the input texture
  iceTexture_t* GetTexture(std::string _directory) override;
private:
  // Loads the input texture, creates its image view and sampler
  iceTexture_t* CreateTexture(std::string _directory);

//=================================================================================================
// HELPERS
//=================================================================================================

  // Wrapper for vulkan's shader module destructor
  void DestroyShaderModule(VkShaderModule& _shader);
  VkFormat FindSupportedFormat(const std::vector<VkFormat>& _formats,
                               VkImageTiling _tiling,
                               VkFormatFeatureFlags _features);
  VkFormat FindDepthFormat();
  u32 FindMemoryType(u32 _mask, VkMemoryPropertyFlags _flags);

//=================================================================================================
// IMAGES
//=================================================================================================

  // Creates a new vkImage and adds it to a new element in the iceImage vector
  u32 CreateImage(u32 _width,
                  u32 _height,
                  VkFormat _format,
                  VkImageTiling _tiling,
                  VkImageUsageFlags _usage,
                  VkMemoryPropertyFlags _memProps);
  // Creates a new vkImageView
  VkImageView CreateImageView(const VkFormat _format,
                              VkImageAspectFlags _aspect,
                              const VkImage& _image);
  // Creates a new vkSampler
  VkSampler CreateSampler();
  // Changes a vkImage's usage tag
  void TransitionImageLayout(VkImage _image,
                             VkFormat _format,
                             VkImageLayout _oldLayout,
                             VkImageLayout _newLayout,
                             VkPipelineStageFlagBits _shaderStage =
                                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

  // Copies data stored in a vkBuffer into a vkImage
  void CopyBufferToImage(VkBuffer _buffer, VkImage _iamge, u32 _width, u32 _height);

  public:
  // NOTE : DELETE -- External classes should not need to get the backend context
  IceRenderContext* GetContext() override
  {
    return rContext;
  };

};

#endif // !ICE_RENDERER_VULKAN_VULKAN_BACKEND_H_
