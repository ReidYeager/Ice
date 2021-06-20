
#include "defines.h"

#if defined(ICE_VULKAN)

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <set>
// TODO : Replace with custom data type -- only used in CreateShader
#include <string>

#include "logger.h"
#include "platform/platform.h"
#include "platform/file_system.h"
#include "renderer/renderer_backend.h"
#include "renderer/backend_context.h"
#include "renderer/buffer.h"
#include "renderer/shader_program.h"
#include "renderer/mesh.h"
#include "renderer/buffer.h"

IceRenderContext rContext;

void RendererBackend::CreateMesh(const char* _model)
{
  mesh_t m = FileSystem::LoadMesh(_model);
  vertexBuffer = CreateAndFillBuffer(m.vertices.data(), sizeof(vertex_t) * m.vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  indexBuffer = CreateAndFillBuffer(m.indices.data(), sizeof(u32) * m.indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
  indexCount = m.indices.size();


}

RendererBackend::RendererBackend()
{
  CreateInstance();
  surface = Platform::CreateSurface(&instance);
  CreateDevice();
  CreateCommandPool(rContext.graphicsCommandPool, rContext.graphicsIdx);
  CreateCommandPool(rContext.transientCommandPool, rContext.transferIdx);

  // TODO : Delete
  mvp.model = glm::mat4(1);
  mvp.view = glm::lookAt(glm::vec3(0.0f, 3.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), {0.0f, 1.0f, 0.0f});
  mvp.projection = glm::perspective(glm::radians(45.0f), 1.778f, 0.1f, 50.0f);
  mvp.projection[1][1] *= -1; // flip the rendered y-axis
  CreateMesh("Cube.obj");

  InitializeComponents();

  //RecordCommandBuffers();
}

RendererBackend::~RendererBackend()
{
  vkDeviceWaitIdle(rContext.device);

  for (iceShaderProgram_t& sp : rContext.shaderPrograms)
  {
    sp.Shutdown();
  }

  // TODO : Delete
  vertexBuffer->FreeBuffer();
  indexBuffer->FreeBuffer();
  mvpBuffer->FreeBuffer();

  for (u32 i = 0; i < MAX_FLIGHT_IMAGE_COUNT; i++)
  {
    vkDestroySemaphore(rContext.device, imageAvailableSemaphores[i], rContext.allocator);
    vkDestroySemaphore(rContext.device, renderCompleteSemaphores[i], rContext.allocator);
    vkDestroyFence(rContext.device, flightFences[i], rContext.allocator);
  }

  vkDestroyDescriptorPool(rContext.device, descriptorPool, rContext.allocator);

  DestroyComponents();

  vkDestroyDescriptorSetLayout(rContext.device, descriptorSetLayout, rContext.allocator);
  vkDestroyCommandPool(rContext.device, rContext.transientCommandPool, rContext.allocator);
  vkDestroyCommandPool(rContext.device, rContext.graphicsCommandPool, rContext.allocator);
  vkDestroyDevice(rContext.device, rContext.allocator);
  vkDestroySurfaceKHR(instance, surface, rContext.allocator);
  vkDestroyInstance(instance, rContext.allocator);
}

void RendererBackend::InitializeComponents()
{
  CreateComponents();

  CreateDescriptorPool();
  CreateSyncObjects();
  CreateCommandBuffers();
}

void RendererBackend::CreateComponents()
{
  CreateSwapchain();
  CreateRenderpass();
  CreateDepthImage();

  CreateFramebuffers();
}

void RendererBackend::DestroyComponents()
{
  // Destroy framebuffers
  for (const auto& fb : frameBuffers)
  {
    vkDestroyFramebuffer(rContext.device, fb, rContext.allocator);
  }
  // Destroy depth image
  vkDestroyImageView(rContext.device, depthImage->view, rContext.allocator);
  vkDestroyImage(rContext.device, depthImage->image, rContext.allocator);
  vkFreeMemory(rContext.device, depthImage->memory, rContext.allocator);

  // Destroy renderpass
  vkDestroyPipeline(rContext.device, pipeline, rContext.allocator);
  vkDestroyPipelineLayout(rContext.device, pipelineLayout, rContext.allocator);
  vkDestroyRenderPass(rContext.device, rContext.renderPass, rContext.allocator);
  // Destroy swapchain
  for (const auto& view : swapchainImageViews)
  {
    vkDestroyImageView(rContext.device, view, rContext.allocator);
  }
  vkDestroySwapchainKHR(rContext.device, swapchain, rContext.allocator); // Implicitly destroys swapchainImages
}

void RendererBackend::RecreateComponents()
{
  // Wait for all frames to complete

  DestroyComponents();
  CreateComponents();
}

void RendererBackend::RenderFrame()
{
  vkWaitForFences(rContext.device, 1, &flightFences[currentFrame], VK_TRUE, UINT64_MAX);

  static float time = 0.0f;
  mvp.model = glm::rotate(glm::mat4(1), (time += 0.001f), glm::vec3(0.0f, 1.0f, 0.0f));
  mvp.model = glm::translate(mvp.model, glm::vec3(0, glm::sin(time * 0.5f) * 0.2f, 0));
  FillBuffer(mvpBuffer->GetMemory(), &mvp, sizeof(mvp));

  u32 imageIndex;
  vkAcquireNextImageKHR(rContext.device, swapchain, UINT64_MAX,
                        imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

  if (imageIsInFlightFences[imageIndex] != VK_NULL_HANDLE)
  {
    vkWaitForFences(rContext.device, 1, &imageIsInFlightFences[imageIndex], VK_TRUE, UINT64_MAX);
  }
  imageIsInFlightFences[imageIndex] = flightFences[currentFrame];

  VkSubmitInfo submitInfo {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &imageAvailableSemaphores[currentFrame];
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &renderCompleteSemaphores[currentFrame];

  vkResetFences(rContext.device, 1, &flightFences[currentFrame]);
  ICE_ASSERT(vkQueueSubmit(rContext.graphicsQueue, 1, &submitInfo, flightFences[currentFrame]),
             "Failed to submit draw command");

  VkPresentInfoKHR presentInfo {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &renderCompleteSemaphores[currentFrame];
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &swapchain;
  presentInfo.pImageIndices = &imageIndex;

  vkQueuePresentKHR(rContext.presentQueue, &presentInfo);

  currentFrame = (currentFrame + 1) % MAX_FLIGHT_IMAGE_COUNT;
}

iceShader_t RendererBackend::CreateShader(const char* _name, IceShaderStageFlags _stage)
{
  iceShader_t s(_name, _stage);

  std::string fileDir = ICE_RESOURCE_SHADER_DIR;
  fileDir.append(_name);
  std::string layoutDir(fileDir);

  switch (_stage)
  {
  case Ice_Shader_Stage_Vert:
    fileDir.append(".vspv");
    layoutDir.append(".vlayout");
    break;
  case Ice_Shader_Stage_Frag:
    fileDir.append(".fspv");
    layoutDir.append(".flayout");
    break;
  case Ice_Shader_Stage_Comp:
    fileDir.append(".cspv");
    layoutDir.append(".clayout");
    break;
  }

  // Load shader file
  std::vector<char> shaderCode = FileSystem::LoadFile(fileDir.c_str());
  //std::vector<char> shaderLayout = Platform::LoadFile(layoutDir);

  // Create VkShaderModule
  VkShaderModuleCreateInfo createInfo {};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = shaderCode.size();
  createInfo.pCode = reinterpret_cast<const u32*>(shaderCode.data());

  ICE_ASSERT(vkCreateShaderModule(rContext.device, &createInfo, rContext.allocator, &s.module),
             "Failed to create shader module");

  // TODO : Get shader bindings from its layout file
  //s.CreateBindings(shaderLayout);

  return s;
}

void RendererBackend::DestroyShaderModule(VkShaderModule& _module)
{
  vkDestroyShaderModule(rContext.device, _module, rContext.allocator);
}

IceBuffer* RendererBackend::CreateAndFillBuffer(const void* _data, VkDeviceSize _size, VkBufferUsageFlags _usage)
{
  IceBuffer* stagingBuffer = CreateBuffer(_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  FillBuffer(stagingBuffer->GetMemory(), _data, _size);

  IceBuffer* buffer = CreateBuffer(_size, _usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  CopyBuffer(stagingBuffer->GetBuffer(), buffer->GetBuffer(), _size);

  vkFreeMemory(rContext.device, stagingBuffer->GetMemory(), rContext.allocator);
  vkDestroyBuffer(rContext.device, stagingBuffer->GetBuffer(), rContext.allocator);

  return buffer;
}

IceBuffer* RendererBackend::CreateBuffer(VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _memProperties)
{
  IceBuffer* buffer = new IceBuffer();
  buffer->AllocateBuffer(_size, _usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, _memProperties);
  buffer->Bind();

  return buffer;
}

void RendererBackend::FillBuffer(VkDeviceMemory _mem, const void* _data, VkDeviceSize _size)
{
  void* tmpData;
  vkMapMemory(rContext.device, _mem, 0, _size, 0, &tmpData);
  memcpy(tmpData, _data, static_cast<size_t>(_size));
  vkUnmapMemory(rContext.device, _mem);
}

void RendererBackend::CopyBuffer(VkBuffer _src, VkBuffer _dst, VkDeviceSize _size)
{
  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = rContext.transientCommandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer transferCommand;
  ICE_ASSERT(vkAllocateCommandBuffers(rContext.device, &allocInfo, &transferCommand),
    "Failed to create transient command buffer");

  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vkBeginCommandBuffer(transferCommand, &beginInfo);

  VkBufferCopy region = {};
  region.size = _size;
  region.dstOffset = 0;
  region.srcOffset = 0;

  vkCmdCopyBuffer(transferCommand, _src, _dst, 1, &region);

  vkEndCommandBuffer(transferCommand);

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &transferCommand;

  vkQueueSubmit(rContext.transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(rContext.transferQueue);

  vkFreeCommandBuffers(rContext.device, rContext.transientCommandPool, 1, &transferCommand);
}

void RendererBackend::DestroyBuffer(VkBuffer _buffer, VkDeviceMemory _memory)
{
  vkFreeMemory(rContext.device, _memory, rContext.allocator);
  vkDestroyBuffer(rContext.device, _buffer, rContext.allocator);
}

void RendererBackend::RecordCommandBuffers()
{
  CreateDescriptorSet(rContext.shaderPrograms[0]);
  iceShaderProgram_t* shaderProgram = nullptr;

  u32 commandCount = static_cast<u32>(commandBuffers.size());
  VkCommandBufferBeginInfo beginInfo {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  VkClearValue clearValues [2] = {};
  clearValues[0].color = { 0.20784313725f, 0.21568627451f, 0.21568627451f, 1.0f };
  clearValues[1].depthStencil = { 1, 0 };

  VkRenderPassBeginInfo rpBeginInfo {};
  rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  rpBeginInfo.renderPass = rContext.renderPass;
  rpBeginInfo.clearValueCount = 2;
  rpBeginInfo.pClearValues = clearValues;
  rpBeginInfo.renderArea.extent = rContext.renderExtent;
  rpBeginInfo.renderArea.offset = { 0, 0 };

  VkDeviceSize offset[] = { 0 };

  shaderProgram = &rContext.shaderPrograms[0];
  for (u32 i = 0; i < commandCount; i++)
  {
    rpBeginInfo.framebuffer = frameBuffers[i];

    ICE_ASSERT(vkBeginCommandBuffer(commandBuffers[i], &beginInfo),
               "Failed to being command buffer");

    vkCmdBeginRenderPass(commandBuffers[i], &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, shaderProgram->GetPipeline());
    vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, shaderProgram->pipelineLayout,
                            0, 1, &shaderProgram->descriptorSet, 0, nullptr);
    vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffer->GetBufferPtr(), offset);
    vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commandBuffers[i], indexCount, 1, 0, 0, 0);
    //vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffers[i]);

    ICE_ASSERT(vkEndCommandBuffer(commandBuffers[i]), "Failed to record command buffer");
  }
}

void RendererBackend::CreateInstance()
{
  // Basic application metadata
  VkApplicationInfo appInfo {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.apiVersion = VK_API_VERSION_1_2;
  appInfo.pEngineName = "Ice";
  appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
  // TODO : Make this editable from the client
  appInfo.pApplicationName = "Ice Application";
  appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);

  // Create the vkInstance
  VkInstanceCreateInfo createInfo {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;
  createInfo.enabledExtensionCount = (u32)instanceExtensions.size();
  createInfo.ppEnabledExtensionNames = instanceExtensions.data();
  createInfo.enabledLayerCount = (u32)deviceLayers.size();
  createInfo.ppEnabledLayerNames = deviceLayers.data();

  ICE_ASSERT(vkCreateInstance(&createInfo, rContext.allocator, &instance),
             "Failed to create instance");
}

void RendererBackend::CreateDevice()
{
  // Choose a physical device
  u32 queueIndices[3];
  u32 queueCount = 3;
  VkPhysicalDevice chosenPhysicalDevice = VK_NULL_HANDLE;
  ChoosePhysicalDevice(chosenPhysicalDevice, queueIndices[0], queueIndices[1], queueIndices[2]);

  IcePrint("Graphics : %u\nPresent : %u\nTransfer : %u\n",
         queueIndices[0], queueIndices[1], queueIndices[2]);

  // Fill physical device details
  rContext.gpu.device = chosenPhysicalDevice;
  rContext.graphicsIdx = queueIndices[0];
  rContext.presentIdx  = queueIndices[1];
  rContext.transferIdx = queueIndices[2];

  IcePhysicalDeviceInformation& dInfo = rContext.gpu;
  VkPhysicalDevice& pDevice = rContext.gpu.device;

  u32 count = 0;
  vkGetPhysicalDeviceFeatures(pDevice, &dInfo.features);
  vkGetPhysicalDeviceProperties(pDevice, &dInfo.properties);
  vkGetPhysicalDeviceMemoryProperties(pDevice, &dInfo.memProperties);
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pDevice, surface, &dInfo.surfaceCapabilities);

  vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &count, nullptr);
  dInfo.queueFamilyProperties.resize(count);
  vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &count, dInfo.queueFamilyProperties.data());

  vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, surface, &count, nullptr);
  dInfo.presentModes.resize(count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, surface, &count, dInfo.presentModes.data());

  vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, surface, &count, nullptr);
  dInfo.surfaceFormats.resize(count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, surface, &count, dInfo.surfaceFormats.data());

  // Create logical device

  VkPhysicalDeviceFeatures enabledFeatures {};
  enabledFeatures.samplerAnisotropy = VK_TRUE;

  const float priority = 1.0f;
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(queueCount);
  for (u32 i = 0; i < queueCount; i++)
  {
    queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[i].queueFamilyIndex = queueIndices[i];
    queueCreateInfos[i].queueCount = 1;
    queueCreateInfos[i].pQueuePriorities = &priority;
  }

  VkDeviceCreateInfo createInfo {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pEnabledFeatures = &enabledFeatures;
  createInfo.enabledExtensionCount   = static_cast<u32>(deviceExtensions.size());
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();
  createInfo.enabledLayerCount   = static_cast<u32>(deviceLayers.size());
  createInfo.ppEnabledLayerNames = deviceLayers.data();
  createInfo.queueCreateInfoCount = queueCount;
  createInfo.pQueueCreateInfos    = queueCreateInfos.data();

  ICE_ASSERT(vkCreateDevice(pDevice, &createInfo, rContext.allocator, &rContext.device),
             "Failed to create vkDevice");

  vkGetDeviceQueue(rContext.device, rContext.graphicsIdx, 0, &rContext.graphicsQueue);
  vkGetDeviceQueue(rContext.device, rContext.presentIdx , 0, &rContext.presentQueue );
  vkGetDeviceQueue(rContext.device, rContext.transferIdx, 0, &rContext.transferQueue);
}

void RendererBackend::ChoosePhysicalDevice(VkPhysicalDevice& _selectedDevice, u32& _graphicsIndex,
                                         u32& _presentIndex, u32& _transferIndex)
{
  // Get all available physical devices
  u32 deviceCount;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  std::vector<VkPhysicalDevice> physDevices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, physDevices.data());

  u32 propertyCount;

  struct BestGPU
  {
    VkPhysicalDevice device;
    u32 graphicsIndex;
    u32 presentIndex;
    u32 transferIndex;
  };
  BestGPU bestFit {};

  for (const auto& pdevice : physDevices)
  {
    vkGetPhysicalDeviceQueueFamilyProperties(pdevice, &propertyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueProperties(propertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(pdevice, &propertyCount, queueProperties.data());

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(pdevice, &props);

    u32 extensionCount;
    vkEnumerateDeviceExtensionProperties(pdevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> physicalDeviceExt(extensionCount);
    vkEnumerateDeviceExtensionProperties(pdevice, nullptr, &extensionCount,
      physicalDeviceExt.data());

    std::set<std::string> requiredExtensionSet(deviceExtensions.begin(), deviceExtensions.end());

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(pdevice, &features);

    for (const auto& ext : physicalDeviceExt)
    {
      requiredExtensionSet.erase(ext.extensionName);
    }

    _graphicsIndex = GetQueueIndex(queueProperties, VK_QUEUE_GRAPHICS_BIT);
    _transferIndex = GetQueueIndex(queueProperties, VK_QUEUE_TRANSFER_BIT);
    _presentIndex = GetPresentIndex(&pdevice, propertyCount, _graphicsIndex);

    if (
      features.samplerAnisotropy &&
      requiredExtensionSet.empty() &&
      _graphicsIndex != -1 &&
      _presentIndex != -1 &&
      _transferIndex != -1)
    {
      if (_graphicsIndex == _presentIndex || _graphicsIndex == _transferIndex
        || _presentIndex == _transferIndex)
      {
        bestFit.device = pdevice;
        bestFit.graphicsIndex = _graphicsIndex;
        bestFit.presentIndex = _presentIndex;
        bestFit.transferIndex = _transferIndex;
      }
      else
      {
        _selectedDevice = pdevice;
        return;
      }
    }
  }

  if (bestFit.device != VK_NULL_HANDLE)
  {
    _selectedDevice = bestFit.device;
    _graphicsIndex = bestFit.graphicsIndex;
    _presentIndex = bestFit.presentIndex;
    _transferIndex = bestFit.transferIndex;
    return;
  }

  IcePrint("Failed to find a suitable GPU");
}

void RendererBackend::CreateCommandPool(VkCommandPool& _pool, u32 _queueIndex,
                                        VkCommandPoolCreateFlags _flags /*= 0*/)
{
  VkCommandPoolCreateInfo createInfo {};
  createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  createInfo.queueFamilyIndex = _queueIndex;
  createInfo.flags = _flags;
  ICE_ASSERT(vkCreateCommandPool(rContext.device, &createInfo, rContext.allocator, &_pool),
             "Failed to create command pool for queue family %u", _queueIndex);
}

void RendererBackend::CreateSwapchain()
{
  // Find the best format
  VkSurfaceFormatKHR formatInfo = rContext.gpu.surfaceFormats[0];
  for (const auto& sFormat : rContext.gpu.surfaceFormats)
  {
    if (sFormat.format == VK_FORMAT_R32G32B32A32_SFLOAT
        && sFormat.colorSpace == VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT)
    {
      formatInfo = sFormat;
      break;
    }
  }

  // Find the best present mode
  VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR; // FIFO guaranteed
  for (const auto& pMode : rContext.gpu.presentModes)
  {
    if (pMode == VK_PRESENT_MODE_MAILBOX_KHR)
    {
      presentMode = pMode;
      break;
    }
  }

  // Get the device's extent
  VkSurfaceCapabilitiesKHR& capabilities = rContext.gpu.surfaceCapabilities;
  VkExtent2D extent;
  if (capabilities.currentExtent.width != UINT32_MAX)
  {
    extent = capabilities.currentExtent;
  }
  else
  {
    u32 width, height;
    Platform::GetWindowExtent(width, height);

    // TODO : Replace with clamp function calls
    if (width < capabilities.minImageExtent.width)
    {
      width = capabilities.minImageExtent.width;
    }
    else if (width > capabilities.maxImageExtent.width)
    {
      width = capabilities.maxImageExtent.width;
    }

    if (height < capabilities.minImageExtent.height)
    {
      height = capabilities.minImageExtent.height;
    }
    else if (height > capabilities.maxImageExtent.height)
    {
      height = capabilities.maxImageExtent.height;
    }

    extent.width = width;
    extent.height = height;
  }

  IcePrint("Width: %u -- Height: %u", extent.width, extent.height);

  // Choose an image count
  u32 imageCount = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0 && capabilities.maxImageCount < imageCount)
  {
    imageCount = capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = surface;
  createInfo.clipped = VK_TRUE;
  createInfo.imageArrayLayers = 1;
  createInfo.imageFormat = formatInfo.format;
  createInfo.imageColorSpace = formatInfo.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.minImageCount = imageCount;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  createInfo.presentMode = presentMode;
  if (rContext.graphicsIdx != rContext.presentIdx)
  {
    u32 sharedIndices[] = {rContext.graphicsIdx, rContext.presentIdx};
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = sharedIndices;
  }
  else
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  ICE_ASSERT(vkCreateSwapchainKHR(rContext.device, &createInfo, rContext.allocator, &swapchain),
             "Failed to create swapchain");

  swapchainFormat = formatInfo.format;
  rContext.renderExtent = extent;

  vkGetSwapchainImagesKHR(rContext.device, swapchain, &imageCount, nullptr);
  swapchainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(rContext.device, swapchain, &imageCount, swapchainImages.data());

  swapchainImageViews.resize(imageCount);
  for (u32 i = 0; i < imageCount; i++)
  {
    swapchainImageViews[i] =
        CreateImageView(swapchainFormat, VK_IMAGE_ASPECT_COLOR_BIT, swapchainImages[i]);

    if (swapchainImageViews[i] == VK_NULL_HANDLE)
    {
      IcePrint("Failed to create swapchain image view");
    }
  }
}

void RendererBackend::CreateRenderpass()
{
  VkAttachmentDescription colorDesc {};
  colorDesc.format = swapchainFormat;
  colorDesc.samples = VK_SAMPLE_COUNT_1_BIT;
  colorDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  colorDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  VkAttachmentReference colorRef;
  colorRef.attachment = 0;
  colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription depthDesc {};
  depthDesc.format = FindDepthFormat();
  depthDesc.samples = VK_SAMPLE_COUNT_1_BIT;
  depthDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthRef {};
  depthRef.attachment = 1;
  depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass {};
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorRef;
  subpass.pDepthStencilAttachment = &depthRef;
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

  VkSubpassDependency dependency {};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkAttachmentDescription attachments[] = { colorDesc, depthDesc };
  VkRenderPassCreateInfo creteInfo {};
  creteInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  creteInfo.attachmentCount = 2;
  creteInfo.pAttachments = attachments;
  creteInfo.subpassCount = 1;
  creteInfo.pSubpasses = &subpass;
  creteInfo.dependencyCount = 1;
  creteInfo.pDependencies = &dependency;

  ICE_ASSERT(vkCreateRenderPass(rContext.device, &creteInfo, rContext.allocator, &rContext.renderPass),
             "Failed to create renderpass");
}

void RendererBackend::CreateDepthImage()
{
  VkFormat format = FindDepthFormat();
  u32 imageIdx = CreateImage(
      rContext.renderExtent.width, rContext.renderExtent.height, format, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  depthImage = iceImages[imageIdx];
  depthImage->view = CreateImageView(format, VK_IMAGE_ASPECT_DEPTH_BIT, depthImage->image);
}

void RendererBackend::CreateFramebuffers()
{
  VkFramebufferCreateInfo createInfo {};
  createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  createInfo.renderPass = rContext.renderPass;
  createInfo.layers = 1;
  createInfo.width = rContext.renderExtent.width;
  createInfo.height = rContext.renderExtent.height;

  u32 imageCount = static_cast<u32>(swapchainImages.size());
  frameBuffers.resize(imageCount);

  for (u32 i = 0; i < imageCount; i++)
  {
    VkImageView attachments[] = {swapchainImageViews[i], depthImage->view};
    createInfo.attachmentCount = 2;
    createInfo.pAttachments = attachments;

    ICE_ASSERT(vkCreateFramebuffer(rContext.device, &createInfo, rContext.allocator, &frameBuffers[i]),
               "Failed to create framebuffers");
  }
}

void RendererBackend::CreateDescriptorPool()
{
  VkDescriptorPoolSize poolSizes[2] = {};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = 3;
  poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[1].descriptorCount = 6;

  VkDescriptorPoolCreateInfo createInfo {};
  createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  createInfo.poolSizeCount = 2;
  createInfo.pPoolSizes = poolSizes;
  createInfo.maxSets = 3;

  ICE_ASSERT(vkCreateDescriptorPool(rContext.device, &createInfo, rContext.allocator, &descriptorPool),
             "Failed to create descriptor pool");
}

void RendererBackend::CreateSyncObjects()
{
  imageAvailableSemaphores.resize(MAX_FLIGHT_IMAGE_COUNT);
  renderCompleteSemaphores.resize(MAX_FLIGHT_IMAGE_COUNT);
  flightFences.resize(MAX_FLIGHT_IMAGE_COUNT);
  imageIsInFlightFences.resize(swapchainImages.size(), VK_NULL_HANDLE);

  VkSemaphoreCreateInfo semaphoreInfo {};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo {};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (u32 i = 0; i < MAX_FLIGHT_IMAGE_COUNT; i++)
  {
    ICE_ASSERT(vkCreateFence(rContext.device, &fenceInfo, rContext.allocator, &flightFences[i]),
               "Failed to create fence");
    ICE_ASSERT(vkCreateSemaphore(rContext.device, &semaphoreInfo, rContext.allocator,
               &imageAvailableSemaphores[i]), "Failed to create image semaphore");
    ICE_ASSERT(vkCreateSemaphore(rContext.device, &semaphoreInfo, rContext.allocator,
               &renderCompleteSemaphores[i]), "Failed to create render semaphore");
  }
}

void RendererBackend::CreateCommandBuffers()
{
  u32 bufferCount = static_cast<u32>(swapchainImages.size());
  commandBuffers.resize(bufferCount);

  VkCommandBufferAllocateInfo allocInfo {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = bufferCount;
  allocInfo.commandPool = rContext.graphicsCommandPool;

  ICE_ASSERT(vkAllocateCommandBuffers(rContext.device, &allocInfo, commandBuffers.data()),
             "Failed to allocate command buffers");
}

void RendererBackend::CreateDescriptorSet(iceShaderProgram_t& _shaderProgram)
{
  mvpBuffer = CreateBuffer(sizeof(mvpMatrices), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  FillBuffer(mvpBuffer->GetMemory(), &mvp, sizeof(mvp));

  VkDescriptorSetAllocateInfo allocInfo {};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &_shaderProgram.descriptorSetLayout;

  ICE_ASSERT(vkAllocateDescriptorSets(rContext.device, &allocInfo, &_shaderProgram.descriptorSet),
             "Failed to allocate descriptor set");

  VkDescriptorBufferInfo mvpBufferInfo {};
  mvpBufferInfo.buffer = mvpBuffer->GetBuffer();
  mvpBufferInfo.offset = 0;
  mvpBufferInfo.range = VK_WHOLE_SIZE;

  VkWriteDescriptorSet descWrites[1] = {};
  descWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descWrites[0].dstSet = _shaderProgram.descriptorSet;
  descWrites[0].dstBinding = 0;
  descWrites[0].dstArrayElement = 0;
  descWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descWrites[0].descriptorCount = 1;
  descWrites[0].pBufferInfo = &mvpBufferInfo;
  descWrites[0].pImageInfo = nullptr;
  descWrites[0].pTexelBufferView = nullptr;

  vkUpdateDescriptorSets(rContext.device, 1, descWrites, 0, nullptr);
}

u32 RendererBackend::GetQueueIndex(
    std::vector<VkQueueFamilyProperties>& _queues, VkQueueFlags _flags)
{
  u32 i = 0;
  u32 bestfit = -1;

  for (const auto& props : _queues)
  {
    if ((props.queueFlags & _flags) == _flags)
    {
      // Attempt to avoid queues that share with Graphics
      if ((props.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0 || (_flags & VK_QUEUE_GRAPHICS_BIT))
      {
        return i;
      }
      else
      {
        bestfit = i;
      }
    }

    i++;
  }

  // Returns bestfit (-1 if no bestfit was found)
  return bestfit;
}

u32 RendererBackend::GetPresentIndex(
  const VkPhysicalDevice* _device, u32 _queuePropertyCount, u32 _graphicsIndex)
{
  u32 bestfit = -1;
  VkBool32 supported;


  for (u32 i = 0; i < _queuePropertyCount; i++)
  {
    vkGetPhysicalDeviceSurfaceSupportKHR(*_device, i, surface, &supported);
    if (supported)
    {
      // Attempt to avoid queues that share with Graphics
      if (i != _graphicsIndex)
      {
        return i;
      }
      else
      {
        bestfit = i;
      }
    }
  }

  // Returns bestfit (-1 if no bestfit was found)
  return bestfit;
}

VkImageView RendererBackend::CreateImageView(const VkFormat _format, VkImageAspectFlags _aspect, const VkImage& _image)
{
  VkImageViewCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  createInfo.subresourceRange.levelCount = 1;
  createInfo.subresourceRange.baseMipLevel = 0;
  createInfo.subresourceRange.layerCount = 1;
  createInfo.subresourceRange.baseArrayLayer = 0;
  createInfo.subresourceRange.aspectMask = _aspect;
  createInfo.image = _image;
  createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
  createInfo.format = _format;

  VkImageView createdView;
  if (vkCreateImageView(rContext.device, &createInfo, rContext.allocator, &createdView) != VK_SUCCESS)
  {
    IcePrint("Failed to create image view");
    return VK_NULL_HANDLE;
  }

  return createdView;
}

VkFormat RendererBackend::FindDepthFormat()
{
  return FindSupportedFormat(
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkFormat RendererBackend::FindSupportedFormat(
    const std::vector<VkFormat>& _formats, VkImageTiling _tiling, VkFormatFeatureFlags _features)
{
  for (VkFormat format : _formats)
  {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(rContext.gpu.device, format, &props);

    if (_tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & _features) == _features)
    {
      return format;
    }
    else if (_tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & _features) == _features)
    {
      return format;
    }
  }

  IcePrint("Failed to find a suitable format");
  return _formats[0];
}

u32 RendererBackend::CreateImage(u32 _width, u32 _height, VkFormat _format,
    VkImageTiling _tiling, VkImageUsageFlags _usage, VkMemoryPropertyFlags _memProps)
{
  VkImageCreateInfo createInfo {};
  createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  createInfo.imageType = VK_IMAGE_TYPE_2D;
  createInfo.extent.width = _width;
  createInfo.extent.height = _height;
  createInfo.extent.depth = 1;
  createInfo.mipLevels = 1;
  createInfo.arrayLayers = 1;
  createInfo.format = _format;
  createInfo.tiling = _tiling;
  createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  createInfo.usage = _usage;
  createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  createInfo.flags = 0;

  VkImage vImage;
  if (vkCreateImage(rContext.device, &createInfo, rContext.allocator, &vImage) != VK_SUCCESS)
  {
    IcePrint("Failed to create vkImages");
    return -1;
  }

  // Allocate device memory for image
  VkMemoryRequirements memReqs;
  vkGetImageMemoryRequirements(rContext.device, vImage, &memReqs);

  VkMemoryAllocateInfo allocInfo {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memReqs.size;
  allocInfo.memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, _memProps);

  VkDeviceMemory vMemory;
  if (vkAllocateMemory(rContext.device, &allocInfo, rContext.allocator, &vMemory) != VK_SUCCESS)
  {
    IcePrint("Failed to allocate texture memory");
  }

  // Bind image and device memory
  vkBindImageMemory(rContext.device, vImage, vMemory, 0);

  iceImage_t* image = new iceImage_t();
  image->image = vImage;
  image->memory = vMemory;
  image->format = _format;
  iceImages.push_back(image);

  return static_cast<u32>(iceImages.size() - 1);
}

u32 RendererBackend::FindMemoryType(u32 _mask, VkMemoryPropertyFlags _flags)
{
  const VkPhysicalDeviceMemoryProperties& props = rContext.gpu.memProperties;

  for (u32 i = 0; i < props.memoryTypeCount; i++)
  {
    if (_mask & (1 << i) && (props.memoryTypes[i].propertyFlags & _flags) == _flags)
    {
      return i;
    }
  }

  IcePrint("Failed to find a suitable memory type");
  return -1;
}

#endif // ICE_VULKAN
