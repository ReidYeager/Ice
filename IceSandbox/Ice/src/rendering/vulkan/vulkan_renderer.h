
#ifndef ICE_RENDERING_VULKAN_VULKAN_RENDERER_H_
#define ICE_RENDERING_VULKAN_VULKAN_RENDERER_H_

#include "defines.h"

#include "math/vector.h"
#include "core/camera.h"
#include "rendering/vulkan/vulkan_context.h"
#include "rendering/mesh.h"

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <vector>

class IvkRenderer
{
private:
  // TODO : Remove/move everything in this private section
  struct IvkObject
  {
    IvkMesh mesh;
    IvkBuffer transformBuffer;
    VkDescriptorSet descriptorSet;
  };

  reIvkContext context;
  std::vector<IvkMaterial> materials;
  std::vector<IvkMesh> meshes;
  // This system is slow and will need to be upgraded later.
  std::vector<std::vector<IvkObject>> scene;

  IvkBuffer viewProjBuffer;
  IvkLights tmpLights;
  reIvkImage texture;
  IvkShadow shadow;

  const u32 shadowResolution = 2048;

public:
  // Initializes the components required to render
  b8 Initialize();
  // Destroys the components required to render, materials, and meshes
  b8 Shutdown();
  // Queues and presents a single frame's render
  b8 Render(IceCamera* _camera);

private:
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
  // Creates a command pool
  b8 CreateCommandPool(b8 _createTransient = false);
  // Creates the swapchain, its images, and their views
  b8 CreateSwapchain();
  // Creates the fences and semaphores for CPU/GPU synchronization
  b8 CreateSyncObjects();

  // Creates an image, view, and sampler for the directional light's shadow
  b8 CreateShadowImages();

  // Commands =====

  // Creates a command buffer for each frame
  b8 CreateCommandBuffers();
  // Records the commands to render the scene
  b8 RecordCommandBuffer(u32 _commandIndex);
  // Allocates and begins recording a command buffer
  VkCommandBuffer BeginSingleTimeCommand(VkCommandPool _pool);
  // Ends recording and queues the command buffer
  b8 EndSingleTimeCommand(VkCommandBuffer& _command, VkCommandPool _pool, VkQueue _queue);

  // Descriptors =====

  // Defines the descriptors and sets available for use
  b8 CreateDescriptorPool();
  // Creates a descriptor set and its layout
  b8 CreateDescriptorSet(std::vector<IvkDescriptor>& _descriptors,
                         VkDescriptorSetLayout* _setLayout,
                         VkDescriptorSet* _set);
  // Binds the input values to the descriptor set
  b8 UpdateDescriptorSet(VkDescriptorSet& _set, std::vector<IvkDescriptorBinding> _bindings);
  // Creates the images buffers used by the shadow renderpass

  // Creates the pipelineLayout and set for the global descriptors
  b8 PrepareGlobalDescriptors();
  // Creates the pipelineLayout and set for the shadow descriptors
  b8 PrepareShadowDescriptors();

  // Renderpass =====

  // Defines a renderpass attachment and its reference
  IvkAttachmentDescRef CreateAttachment(IvkAttachmentSettings _settings, u32 _index);
  // Creates the final renderpass that creates the presented image
  b8 CreateRenderpass(VkRenderPass* _renderpass,
                      std::vector<IvkAttachmentSettings> _attachSettings,
                      std::vector<IvkSubpassSettings> _subpasses,
                      std::vector<IvkSubpassDependencies> _dependencies);
  // Creates a framebuffer that binds the input image views for use in the renderpass
  b8 CreateFrameBuffer(VkFramebuffer* _framebuffer,
                       VkRenderPass& _renderpass,
                       VkExtent2D _extents,
                       std::vector<VkImageView> _views);

  // Creates the main camera's depth image and its view
  b8 CreateDepthImage();

  // Creates a frame buffer for each swapchain image
  b8 CreateMainFrameBuffers();
  // Creates a frame buffer for the directional light's shadows
  b8 CreateShadowFrameBuffer();

  // Defines the settings used to create the presentation renderpass
  b8 CreateMainRenderPass();
  // Defines the settings used to create the light depth-buffer renderpass
  b8 CreateShadowRenderpass();

  // Platform =====

  // Retrieves all of the extensions the platform requires to render and present with Vulkan
  void GetPlatformExtensions(std::vector<const char*>& _extensions);
  // Creates a vendor-specific surface for display
  b8 CreateSurface();
  // Returns the window width and height
  vec2U GetPlatformWindowExtents();

  // Material =====

  // Creates a simple pipeline layout
  b8 CreatePipelinelayout(VkPipelineLayout* _pipelineLayout,
                          std::vector<VkDescriptorSetLayout> _layouts,
                          std::vector<VkPushConstantRange> _pushRanges);
  // Creates a presentation pipeline and a shadow-pass pipeline
  b8 CreatePipeline(IvkMaterial& material);
  // Creates a vulkan shader module
  b8 CreateShaderModule(VkShaderModule* _module, const char* _shader);

  // Buffers =====

  // Allocates a new block of memory on the GPU
  b8 CreateBuffer(IvkBuffer* _buffer,
                  u64 _size,
                  VkBufferUsageFlags _usage,
                  VkMemoryPropertyFlags _memoryProperties,
                  void* _data = nullptr);
  // Binds the buffer to the memory starting at the offset
  b8 BindBuffer(IvkBuffer* _buffer, u64 _offset);
  // Fills the given buffer with data
  b8 FillBuffer(IvkBuffer* _buffer, void* _data, u64 _size = 0, u64 _offset = 0);
  // Destroys the input buffer and can free its memory
  void DestroyBuffer(const IvkBuffer* _buffer, b8 _freeMemory = true);

  // Images =====

  // Creates a vulkan image
  b8 CreateImage(reIvkImage* _image,
                 VkExtent2D _extents,
                 VkFormat _format,
                 VkImageUsageFlags _usage);
  void DestroyImage(const reIvkImage* _image);
  // Creates a vulkan image view
  b8 CreateImageView(VkImageView* _view,
                     VkImage _image,
                     VkFormat _format,
                     VkImageAspectFlags _aspectMask);
  // Creates a vulkan image sampler
  b8 CreateImageSampler(reIvkImage* _image);
  // Transitions the image into either the transfer-dst (!forSampling) or shader-read-only layout
  void TransitionImageLayout(reIvkImage* _image,
                             b8 _forSampling,
                             VkPipelineStageFlagBits _shaderStage);
  // Copies the information in the buffer into a vkImage
  void CopyBufferToImage(IvkBuffer* _buffer, reIvkImage* _image);

  // Creates an image, view, and sampler for the input image file
  b8 CreateTexture(reIvkImage* _image, const char* _directory);

  // ====================
  // API
  // ====================
  public:
  // Creates a new material (descriptor set, pipeline layout, pipeline)
  u32 CreateMaterial(const std::vector<IceShaderInfo>& _shaders);
  // Loads the mesh and fills its buffers
  u32 CreateMesh(const char* _directory);
  b8 AddMeshToScene(u32 _meshIndex, u32 _materialIndex);

};

#endif // !define ICE_RENDERING_RE_RENDERER_VULKAN_H_