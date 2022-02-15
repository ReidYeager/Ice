
#ifndef ICE_RENDERING_VULKAN_VULKAN_RENDERER_H_
#define ICE_RENDERING_VULKAN_VULKAN_RENDERER_H_

#include "defines.h"

#include "math/vector.h"
#include "core/camera.h"
#include "core/object.h"
#include "rendering/vulkan/vulkan_context.h"
#include "rendering/render_context.h"
#include "rendering/mesh.h"

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <vector>

// TODO : ~!!~ Restructure the scene graph (0/4)
// [ ] Establish a proper hierarchy in /core/
// [ ] Create a structure in the renderer optimized for rendering
// [ ] Link the core hierarchy to the rendering structure via core
// [ ] Integrate cameras & lights into this scene system

// TODO : Allow source to change material pipeline settings

class IvkRenderer
{
private:
  IvkContext context;

  // TODO : Remove/move all structs/variables defined here.
  struct IvkObject
  {
    IvkMesh mesh;
    IvkBuffer transformBuffer;
    VkDescriptorSet descriptorSet;
  };

  std::vector<IvkTexture> textures;
  std::vector<IvkShader> shaders;
  std::vector<IvkMaterial> materials;
  std::vector<IvkBuffer> materialBuffers;
  std::vector<IvkMesh> meshes;
  // This system is slow and will need to be upgraded later.
  std::vector<std::vector<IvkObject>> scene;

  IvkBuffer globalDescriptorBuffer;
  IvkLights tmpLights;
  //IvkImage texture;
  IvkShadow shadow;

  const u32 shadowResolution = 2048;

public:
  // Initializes the components required to render
  b8 Initialize(const IceRendererSettings& _settings);
  // Destroys the components required to render, materials, and meshes
  b8 Shutdown();
  // Queues and presents a single frame's render
  b8 Render(IceCamera* _camera);

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
  // Creates a command pool
  b8 CreateCommandPool(b8 _createTransient = false);
  // Creates the swapchain, its images, and their views
  b8 CreateSwapchain();
  // Creates the fences and semaphores for CPU/GPU synchronization
  b8 CreateSyncObjects();

  //=========================
  // Commands
  //=========================

  // Creates a command buffer for each frame
  b8 CreateCommandBuffers();
  // Records the commands to render the scene
  b8 RecordCommandBuffer(u32 _commandIndex);
  // Allocates and begins recording a command buffer
  VkCommandBuffer BeginSingleTimeCommand(VkCommandPool _pool);
  // Ends recording and queues the command buffer
  b8 EndSingleTimeCommand(VkCommandBuffer& _command, VkCommandPool _pool, VkQueue _queue);

  //=========================
  // Descriptors
  //=========================

  // Defines the descriptors and sets available for use
  b8 CreateDescriptorPool();
  // Creates a descriptor set and its layout
  b8 CreateDescriptorSetAndLayout(std::vector<IvkDescriptor>& _descriptors,
                                  VkDescriptorSetLayout* _setLayout,
                                  VkDescriptorSet* _set);
  // Binds the input values to the descriptor set
  b8 UpdateDescriptorSet(VkDescriptorSet& _set,
                         std::vector<IvkDescriptorBinding> _bindings,
                         u32 _offset = 0);
  // Creates the images buffers used by the shadow renderpass

  // Creates the pipelineLayout and set for the global descriptors
  b8 PrepareGlobalDescriptors();
  // Creates the pipelineLayout and set for the shadow descriptors
  b8 PrepareShadowDescriptors();

  //=========================
  // Renderpass
  //=========================

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

  // Directional shadow =====

  // Defines the shadow renderpass settings
  b8 CreateShadowRenderpass();
  // Creates a frame buffer for the directional light's shadow
  b8 CreateShadowFrameBuffer();

  // Deferred =====

  b8 CreateDeferredRenderpass();
  // Creates the G-buffers and framebuffer for each swapchain image
  b8 CreateDeferredFramebuffers();

  // Forward =====
  b8 CreateForwardRenderpass();
  b8 CreateForwardFramebuffers();

  //=========================
  // Platform
  //=========================

  // Retrieves all of the extensions the platform requires to render and present with Vulkan
  void GetPlatformExtensions(std::vector<const char*>& _extensions);
  // Creates a vendor-specific surface for display
  b8 CreateSurface();
  // Returns the window width and height
  vec2U GetPlatformWindowExtents();

  //=========================
  // Material
  //=========================

  // Creates a simple pipeline layout
  b8 CreatePipelinelayout(VkPipelineLayout* _pipelineLayout,
                          std::vector<VkDescriptorSetLayout> _layouts,
                          std::vector<VkPushConstantRange> _pushRanges);
  // Creates a presentation pipeline and a shadow-pass pipeline
  b8 CreatePipeline(IvkMaterial& material, IceMaterialTypes _type, u32 _subpass = 0);
  // Creates a vulkan shader module
  b8 CreateShaderModule(VkShaderModule* _module, const char* _shader);

  b8 CreateMaterial(IvkMaterial* _newMaterial,
                    const std::vector<IceHandle>& _shaders,
                    std::vector<IceShaderDescriptor>& _materialDescriptors,
                    IceMaterialTypes _type,
                    u32 _subpassIndex = 0);

  //=========================
  // Buffer
  //=========================

  // Allocates a new block of memory on the GPU
  b8 CreateBuffer(IvkBuffer* _buffer,
                  u64 _size,
                  VkBufferUsageFlags _usage,
                  VkMemoryPropertyFlags _memoryProperties,
                  void* _data = nullptr);
  // Binds the buffer to the memory starting at the offset
  b8 BindBufferAndMemory(IvkBuffer* _buffer, u64 _offset);
  // Destroys the input buffer and can free its memory
  void DestroyBuffer(const IvkBuffer* _buffer, b8 _freeMemory = true);

  //=========================
  // Image
  //=========================

  // Creates a vulkan image
  b8 CreateImage(IvkImage* _image,
                 VkExtent2D _extents,
                 VkFormat _format,
                 VkImageUsageFlags _usage);
  void DestroyImage(const IvkImage* _image);
  // Creates a vulkan image view
  b8 CreateImageView(VkImageView* _view,
                     VkImage _image,
                     VkFormat _format,
                     VkImageAspectFlags _aspectMask);
  // Creates a vulkan image sampler
  b8 CreateImageSampler(IvkImage* _image);
  // Transitions the image into either the transfer-dst (!forSampling) or shader-read-only layout
  void TransitionImageLayout(IvkImage* _image,
                             b8 _forSampling,
                             VkPipelineStageFlagBits _shaderStage);
  // Copies the information in the buffer into a vkImage
  void CopyBufferToImage(IvkBuffer* _buffer, IvkImage* _image);

  // Creates an image, view, and sampler for the input image file
  b8 LoadTextureFile(IvkTexture* _texture);
  VkFormat GetDepthFormat();

  //=========================
  // GUI
  //=========================
  b8 InitImgui();
  void ShutdownImgui();
  void RenderImgui(VkCommandBuffer& _cmdBuffer);

  // ====================
  public: // API
  // ====================

  // Creates a new material (descriptor set, pipeline layout, pipeline)
  IceHandle CreateNewMaterial(const std::vector<IceHandle>& _shaders,
                              std::vector<IceShaderDescriptor>& _materialDescriptors,
                              IceMaterialTypes _type,
                              u32 _subpassIndex = 0);
  IceHandle CreateShader(const std::string _dir, const IceShaderStage _stage);
  // Destroys and re-creates the shader module
  b8 RecreateShader(const IceShader& _shader);
  // Destroys and re-creates the material's pipelines
  b8 RecreateMaterial(IceHandle _backendMaterial,
                      const std::vector<IceHandle>& _shaders,
                      std::vector<IceShaderDescriptor>& _descriptors);
  IceHandle GetTexture(const char* _directory, IceImageType _type);
  void AssignMaterialTextures(IceMaterial& _material, std::vector<IceHandle> _textures);
  b8 SetDeferredLightingMaterial(IceHandle _material);
  b8 FillMaterialBuffer(IceHandle _buffer, void* _data);
  void DestroyMaterial(IceHandle _material);

  // Loads the mesh and fills its buffers
  u32 CreateMesh(const char* _directory);
  b8 AddMeshToScene(IceObject* _object);
  // Fills the given buffer with data
  b8 FillBuffer(IvkBuffer* _buffer, void* _data, u64 _size = 0, u64 _offset = 0);

};

#endif // !define ICE_RENDERING_RE_RENDERER_VULKAN_H_
