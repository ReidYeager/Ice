
#ifndef RENDERER_RENDERER_BACKEND_H
#define RENDERER_RENDERER_BACKEND_H 1

#include "defines.h"
#include "renderer/backend_context.h"
#include "renderer/shader_program.h"
#include "renderer/mesh.h"
#include "renderer/buffer.h"

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include <string.h>

struct iceTexture_t
{
  u32 imageIndex;
  // TODO : Fix const char* directory being lost when texture vector expands
  std::string directory;

  iceTexture_t(std::string _dir) : imageIndex(0), directory(_dir) {}
};

struct IceRenderPacket
{
  glm::mat4 viewMatrix;
  glm::mat4 projectionMatrix;
};

class RendererBackend
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

  void CreateMesh(const char* _directory);
  // TODO : Replace string with const char* -- text was lost when using char*'s
  u32 GetTexture(std::string _directory);
  u32 CreateTexture(std::string _directory);

//=================================================================================================
// VARIABLES
//=================================================================================================
private:
  //IceRenderContext rContext;

  std::vector<const char*> deviceLayers =
      { "VK_LAYER_KHRONOS_validation" };
  std::vector<const char*> instanceExtensions = 
      { VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_win32_surface" };
  std::vector<const char*> deviceExtensions =
      { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

  std::vector<iceImage_t*> iceImages;
  std::vector<iceTexture_t*> iceTextures;

public:
  bool shouldResize = false;

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

  void RenderFrame(IceRenderPacket* _packet);

  // Creates a new iceShader
  iceShader_t CreateShader(const char* _name, IceShaderStageFlags _stage);
  // TODO : I don't like this. Find a better way to destroy shaders
  // TODO : API CULL MARK
  void DestroyShaderModule(VkShaderModule& _shader);

  // Creates a buffer on the GPU and fills it with data
  
  // TODO : API CULL MARK
  IceBuffer* CreateAndFillBuffer(const void* _data, VkDeviceSize _size, VkBufferUsageFlags _usage);
  // TODO : API CULL MARK
  IceBuffer* CreateBuffer(VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _memProperties);
  void FillBuffer(VkDeviceMemory _mem, const void* _data, VkDeviceSize _size);
  // TODO : API CULL MARK
  void CopyBuffer(VkBuffer _src, VkBuffer _dst, VkDeviceSize _size);
  // TODO : API CULL MARK
  void DestroyBuffer(VkBuffer _buffer, VkDeviceMemory _memory);

  // TODO : API CULL MARK?
  void RecordCommandBuffers();

  // TODO : Move to shader programs?
  void CreateDescriptorSet(iceShaderProgram_t& _shaderProgram);

private:
  // Calls CreateComponents
  // Creates components that live through recreation
  void InitializeComponents();

  void CreateInstance();
  void CreateLogicalDevice();
  void ChoosePhysicalDevice();
  void FillPhysicalDeviceInformation();
  // TODO : API CULL MARK
  void CreateCommandPool(VkCommandPool& _pool, u32 _queueIndex,
                         VkCommandPoolCreateFlags _flags = 0);

  // TODO : API CULL MARK? -- Ignore if OpenGL/DirectX utilize similar objects
  // Render components
  void CreateSwapchain();
  void CreateRenderpass();
  void CreateDepthImage();
  void CreateFramebuffers();

  void CreateDescriptorPool();
  void CreateSyncObjects();
  void CreateCommandBuffers();

  // ===== Helpers =====
  // Returns the first instance of a queue with the input flags
  // TODO : API CULL MARK
  u32 GetQueueIndex(std::vector<VkQueueFamilyProperties>& _queues, VkQueueFlags _flags);
  // Returns the first instance of a presentation queue
  // TODO : API CULL MARK
  u32 GetPresentIndex(
      const VkPhysicalDevice* _device, u32 _queuePropertyCount, u32 _graphicsIndex);
  VkFormat FindDepthFormat();
  // TODO : API CULL MARK
  VkFormat FindSupportedFormat(const std::vector<VkFormat>& _formats,
                               VkImageTiling _tiling, VkFormatFeatureFlags _features);
  u32 FindMemoryType(u32 _mask, VkMemoryPropertyFlags _flags);

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

};

#endif // !RENDERER_RENDER_BACKEND_H
