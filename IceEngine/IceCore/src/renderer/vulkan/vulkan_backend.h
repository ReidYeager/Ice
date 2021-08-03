
#ifndef ICE_RENDERER_VULKAN_VULKAN_BACKEND_H_
#define ICE_RENDERER_VULKAN_VULKAN_BACKEND_H_

#include "defines.h"
#include "renderer/vulkan/vulkan_context.h"
#include "renderer/renderer_backend.h"

class VulkanBackend : public RendererBackend
{
private:
  // TODO : Delete
  struct mvpMatrices
  {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
  } mvp;
  IceBuffer* mvpBuffer;

  IceBuffer* vertexBuffer;
  IceBuffer* indexBuffer;
  u32 indexCount = 0;

//=================================================================================================
// VARIABLES
//=================================================================================================
private:
  IceRenderContext* rContext;

  std::vector<const char*> deviceLayers =
      { "VK_LAYER_KHRONOS_validation" };
  std::vector<const char*> instanceExtensions = 
      { VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_win32_surface" };
  std::vector<const char*> deviceExtensions =
      { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

  std::vector<iceImage_t*> iceImages;
  std::vector<iceTexture_t*> iceTextures;

  bool shouldResize = false;

//=================================================================================================
// FUNCTIONS
//=================================================================================================
public:
  //// Initializes the fundamentals required to create renderer components
  //VulkanBackend();
  //// Destroys all components used for rendering & presentation
  //~VulkanBackend();
  void Initialize() override;
  void Shutdown() override;

  void RenderFrame(IceRenderPacket* _packet) override;
  void RecordCommandBuffers() override;
  void Resize(u32 _width = 0, u32 _height = 0) override;

private:
//=================================================================================================
// Rendering component management
//=================================================================================================

  // Calls CreateComponents
  // Creates components that live through recreation
  void InitializeComponents();
  // Creates components required to render images
  void CreateComponents();
  // Destroys rendering components
  void DestroyComponents();
  // Destroys and recreates rendering components
  void RecreateComponents();

//=================================================================================================
// Static rendering components
//=================================================================================================

  void InitializeAPI();
  void CreateLogicalDevice();
  void CreateDescriptorPool();
  void CreateSyncObjects();
  // TODO : API CULL MARK
  void CreateCommandPool(VkCommandPool& _pool, u32 _queueIndex,
    VkCommandPoolCreateFlags _flags = 0);
  // TODO : API CULL MARK?

//=================================================================================================
// Rendering components
//=================================================================================================

  // TODO : API CULL MARK? -- Ignore if OpenGL/DirectX utilize similar objects
  // Render components
  void CreateSwapchain();
  void CreateRenderpass();
  void CreateDepthImage();
  void CreateFramebuffers();
  void CreateCommandBuffers();

//=================================================================================================
// GPU
//=================================================================================================

  void ChoosePhysicalDevice();
  void FillPhysicalDeviceInformation();
  // Returns the first instance of a queue with the input flags
  // TODO : API CULL MARK
  u32 GetQueueIndex(std::vector<VkQueueFamilyProperties>& _queues, VkQueueFlags _flags);
  // Returns the first instance of a presentation queue
  // TODO : API CULL MARK
  u32 GetPresentIndex(
    const VkPhysicalDevice* _device, u32 _queuePropertyCount, u32 _graphicsIndex);

//=================================================================================================
// NON-API HELPERS
//=================================================================================================

public:
  mesh_t CreateMesh(const char* _directory) override;
private:
  // TODO : Replace string with const char* -- text was lost when using char*'s
  u32 GetTexture(std::string _directory);
  u32 CreateTexture(std::string _directory);

//=================================================================================================
// HELPERS
//=================================================================================================

  // TODO : I don't like this. Find a better way to destroy shaders
  // TODO : API CULL MARK
  void DestroyShaderModule(VkShaderModule& _shader);
  // TODO : API CULL MARK
  VkFormat FindSupportedFormat(const std::vector<VkFormat>& _formats,
    VkImageTiling _tiling, VkFormatFeatureFlags _features);
  VkFormat FindDepthFormat();
  u32 FindMemoryType(u32 _mask, VkMemoryPropertyFlags _flags);

//=================================================================================================
// BUFFERS
//=================================================================================================

  // Creates a buffer on the GPU and fills it with data
  // TODO : API CULL MARK
  IceBuffer* CreateAndFillBuffer(const void* _data, VkDeviceSize _size, VkBufferUsageFlags _usage);
  // TODO : API CULL MARK
  IceBuffer* CreateBuffer(
      VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _memProperties);
  void FillBuffer(VkDeviceMemory _mem, const void* _data, VkDeviceSize _size);
  // TODO : API CULL MARK
  void CopyBuffer(VkBuffer _src, VkBuffer _dst, VkDeviceSize _size);
  // TODO : API CULL MARK
  void DestroyBuffer(VkBuffer _buffer, VkDeviceMemory _memory);

//=================================================================================================
// IMAGES
//=================================================================================================

  // TODO : Move to an image manager?
  // TODO : API CULL MARK
  u32 CreateImage(u32 _width, u32 _height, VkFormat _format, VkImageTiling _tiling,
    VkImageUsageFlags _usage, VkMemoryPropertyFlags _memProps);
  // TODO : API CULL MARK
  VkImageView CreateImageView(
    const VkFormat _format, VkImageAspectFlags _aspect, const VkImage& _image);
  // TODO : API CULL MARK
  VkSampler CreateSampler();
  // TODO : API CULL MARK
  void TransitionImageLayout(
      VkImage _image, VkFormat _format, VkImageLayout _oldLayout, VkImageLayout _newLayout,
      VkPipelineStageFlagBits _shaderStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    // TODO : API CULL MARK
  void CopyBufferToImage(VkBuffer _buffer, VkImage _iamge, u32 _width, u32 _height);

//=================================================================================================
// SHADERS
//=================================================================================================
  // Creates a new iceShader
  iceShader_t CreateShader(const char* _name, IceShaderStageFlags _stage);

  public:
  // TODO : Move to shader programs?
  void CreateDescriptorSet(u32 _programIndex);

  // TODO : DELETE
  IceRenderContext* GetContext() override
  {
    return rContext;
  };

};

#endif // !ICE_RENDERER_VULKAN_VULKAN_BACKEND_H_
