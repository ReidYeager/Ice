
#ifndef ICE_RENDERER_VULKAN_VULKAN_BACKEND_H_
#define ICE_RENDERER_VULKAN_VULKAN_BACKEND_H_

#include "asserts.h"
#include "defines.h"

#include "renderer/vulkan/vulkan_context.h"
#include "renderer/vulkan/vulkan_material.h"
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
  IvkBuffer* mvpBuffer;

  IvkBuffer* vertexBuffer;
  IvkBuffer* indexBuffer;
  u32 indexCount = 0;

//=================================================================================================
// VARIABLES
//=================================================================================================
private:
  IceRenderContext* rContext;

  std::vector<iceImage_t*> iceImages;
  std::vector<iceTexture_t*> iceTextures;

  bool shouldResize = false;

//=================================================================================================
// FUNCTIONS
//=================================================================================================
public:
  // Initializes the fundamentals required to create renderer components
  void Initialize() override;
  // Destroys all components used for rendering & presentation
  void Shutdown() override;

  void RenderFrame(IceRenderPacket* _packet) override;
  void RecordCommandBuffers(IvkMaterial* _shader) override;
  void Resize(u32 _width = 0, u32 _height = 0) override;

// DELETE
//=================================================================================================
  std::vector<IceMaterial*> materials;
  IvkBuffer* GetMVPBuffer() { return mvpBuffer; }
  mvpMatrices* GetMVP() { return &mvp; }
  void AddMaterial(IceMaterial* _material) { materials.push_back(_material); }

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

  void GetRequiredPlatformExtensions(std::vector<const char*>& _extensions);
  void CreateSurface();
  VkExtent2D GetWindowExtent();

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
  u32 GetQueueIndex(std::vector<VkQueueFamilyProperties>& _queues, VkQueueFlags _flags);
  // Returns the first instance of a presentation queue
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
  void DestroyShaderModule(VkShaderModule& _shader);
  VkFormat FindSupportedFormat(const std::vector<VkFormat>& _formats,
    VkImageTiling _tiling, VkFormatFeatureFlags _features);
  VkFormat FindDepthFormat();
  u32 FindMemoryType(u32 _mask, VkMemoryPropertyFlags _flags);

//=================================================================================================
// IMAGES
//=================================================================================================

  // TODO : Move to an image manager?
  u32 CreateImage(u32 _width, u32 _height, VkFormat _format, VkImageTiling _tiling,
    VkImageUsageFlags _usage, VkMemoryPropertyFlags _memProps);
  VkImageView CreateImageView(
    const VkFormat _format, VkImageAspectFlags _aspect, const VkImage& _image);
  VkSampler CreateSampler();
  void TransitionImageLayout(
      VkImage _image, VkFormat _format, VkImageLayout _oldLayout, VkImageLayout _newLayout,
      VkPipelineStageFlagBits _shaderStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

  void CopyBufferToImage(VkBuffer _buffer, VkImage _iamge, u32 _width, u32 _height);

//=================================================================================================
// SHADERS
//=================================================================================================
  // Creates a new iceShader
  IceShader CreateShader(const char* _name, IceShaderStageFlags _stage);

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
