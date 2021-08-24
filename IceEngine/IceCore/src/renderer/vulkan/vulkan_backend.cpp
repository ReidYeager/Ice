
#include "defines.h"
#include "logger.h"

#include "renderer/vulkan/vulkan_backend.h"
#include "renderer/vulkan/vulkan_context.h"
#include "renderer/vulkan/vulkan_buffer.h"
#include "renderer/vulkan/vulkan_buffer.h"
#include "renderer/buffer.h"
#include "renderer/mesh.h"
#include "renderer/shader_program.h"
#include "platform/platform.h"
#include "platform/file_system.h"

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <set>
// TODO : Replace with custom data type -- only used in CreateShader
#include <string>

bool WindowResizeCallback(u16 _eventCode, void* _sender, void* _listener, IceEventData _data);

void VulkanBackend::Initialize()
{
  rContext = new IceRenderContext();

  EventManager.Register(Ice_Event_Window_Resized, this, WindowResizeCallback);

  InitializeAPI();
  CreateSurface();
  ChoosePhysicalDevice();
  FillPhysicalDeviceInformation();
  CreateLogicalDevice();
  CreateCommandPool(rContext->graphicsCommandPool, rContext->graphicsIdx,
                    VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
  CreateCommandPool(rContext->transientCommandPool, rContext->transferIdx);

  // TODO : Delete
  mvpBuffer = CreateBuffer(rContext, sizeof(mvpMatrices), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  FillBuffer(rContext, mvpBuffer->GetMemory(), &mvp, sizeof(mvp));
  mvp.model = glm::mat4(1);

  InitializeComponents();

  DefineRenderFrame(VulkanBackend::RenderFrame);
  DefineRecordCommandBuffers(VulkanBackend::RecordCommandBuffers);
}

void VulkanBackend::Shutdown()
{
  vkDeviceWaitIdle(rContext->device);

  // TODO : Delete
  vertexBuffer->FreeBuffer(rContext);
  delete(vertexBuffer);
  indexBuffer->FreeBuffer(rContext);
  delete(indexBuffer);
  mvpBuffer->FreeBuffer(rContext);
  delete(mvpBuffer);

  for (u32 i = 0; i < MAX_FLIGHT_IMAGE_COUNT; i++)
  {
    vkDestroySemaphore(rContext->device, rContext->syncObjects.imageAvailableSemaphores[i],
      rContext->allocator);
    vkDestroySemaphore(rContext->device, rContext->syncObjects.renderCompleteSemaphores[i],
      rContext->allocator);
    vkDestroyFence(rContext->device, rContext->syncObjects.flightFences[i], rContext->allocator);
  }

  vkDestroyDescriptorPool(rContext->device, rContext->descriptorPool, rContext->allocator);

  DestroyComponents();

  ShadersShutdown(rContext);

  for (iceImage_t* i : iceImages)
  {
    if (i->image != VK_NULL_HANDLE)
    {
      vkDestroyImageView(rContext->device, i->view, rContext->allocator);
      vkDestroyImage(rContext->device, i->image, rContext->allocator);
      vkFreeMemory(rContext->device, i->memory, rContext->allocator);
      vkDestroySampler(rContext->device, i->sampler, rContext->allocator);
      i->image = VK_NULL_HANDLE;
    }
    delete(i);
  }
  for (iceTexture_t* t : iceTextures)
  {
    delete(t);
  }

  vkDestroyCommandPool(rContext->device, rContext->transientCommandPool, rContext->allocator);
  vkDestroyCommandPool(rContext->device, rContext->graphicsCommandPool, rContext->allocator);
  vkDestroyDevice(rContext->device, rContext->allocator);
  vkDestroySurfaceKHR(rContext->instance, rContext->surface, rContext->allocator);
  vkDestroyInstance(rContext->instance, rContext->allocator);

  delete(rContext);
}

// TODO : Split into render call & presentation steps
void VulkanBackend::RenderFrame(IceRenderPacket* _packet)
{
  if (shouldResize)
  {
    RecreateComponents();
    shouldResize = false;
  }

  u32& currentFrame = rContext->syncObjects.currentFrame;
  vkWaitForFences(rContext->device, 1, &rContext->syncObjects.flightFences[currentFrame],
    VK_TRUE, UINT64_MAX);

  static float time = 0.0f;
  mvp.model = glm::rotate(glm::mat4(1), glm::radians((time += _packet->deltaTime) * 45), glm::vec3(0.0f, 1.0f, 0.0f));
  mvp.model = glm::translate(mvp.model, glm::vec3(0, glm::sin(time * 2.0f) * 0.2f, 0));
  mvp.view = _packet->viewMatrix;
  mvp.projection = _packet->projectionMatrix;
  FillBuffer(rContext, mvpBuffer->GetMemory(), &mvp, sizeof(mvp));

  u32 imageIndex;
  VkResult result = vkAcquireNextImageKHR(rContext->device, rContext->swapchain, UINT64_MAX,
    rContext->syncObjects.imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR)
  {
    RecreateComponents();
    return;
  }
  else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
  {
    throw "Failed to acquire swapchain image";
  }

  if (rContext->syncObjects.imageIsInFlightFences[imageIndex] != VK_NULL_HANDLE)
  {
    vkWaitForFences(rContext->device, 1, &rContext->syncObjects.imageIsInFlightFences[imageIndex],
      VK_TRUE, UINT64_MAX);
  }
  rContext->syncObjects.imageIsInFlightFences[imageIndex] = 
      rContext->syncObjects.flightFences[currentFrame];

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &rContext->syncObjects.imageAvailableSemaphores[currentFrame];
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &rContext->commandBuffers[imageIndex];
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &rContext->syncObjects.renderCompleteSemaphores[currentFrame];

  vkResetFences(rContext->device, 1, &rContext->syncObjects.flightFences[currentFrame]);
  IVK_ASSERT(vkQueueSubmit(rContext->graphicsQueue, 1, &submitInfo,
                           rContext->syncObjects.flightFences[currentFrame]),
    "Failed to submit draw command");

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &rContext->syncObjects.renderCompleteSemaphores[currentFrame];
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &rContext->swapchain;
  presentInfo.pImageIndices = &imageIndex;

  result = vkQueuePresentKHR(rContext->presentQueue, &presentInfo);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || shouldResize)
  {
    shouldResize = false;
    RecreateComponents();
  }
  else if (result != VK_SUCCESS)
  {
    throw "Failed to present swapchain";
  }

  rContext->syncObjects.currentFrame = (currentFrame + 1) % MAX_FLIGHT_IMAGE_COUNT;
}

void VulkanBackend::RecordCommandBuffers(IvkMaterial* _shader)
{
  u32 commandCount = static_cast<u32>(rContext->commandBuffers.size());
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  VkClearValue clearValues[2] = {};
  clearValues[0].color = { 0.20784313725f, 0.21568627451f, 0.21568627451f, 1.0f };
  clearValues[1].depthStencil = { 1, 0 };

  VkRenderPassBeginInfo rpBeginInfo{};
  rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  rpBeginInfo.renderPass = rContext->renderPass;
  rpBeginInfo.clearValueCount = 2;
  rpBeginInfo.pClearValues = clearValues;
  rpBeginInfo.renderArea.extent = rContext->renderExtent;
  rpBeginInfo.renderArea.offset = { 0, 0 };

  VkDeviceSize offset[] = { 0 };

  for (u32 i = 0; i < commandCount; i++)
  {
    rpBeginInfo.framebuffer = rContext->frameBuffers[i];

    IVK_ASSERT(vkBeginCommandBuffer(rContext->commandBuffers[i], &beginInfo),
      "Failed to being command buffer");

    vkCmdBeginRenderPass(rContext->commandBuffers[i], &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    _shader->Render(rContext->commandBuffers[i]);
    vkCmdBindVertexBuffers(rContext->commandBuffers[i], 0, 1, vertexBuffer->GetBufferPtr(), offset);
    vkCmdBindIndexBuffer(rContext->commandBuffers[i], indexBuffer->GetBuffer(), 0,
      VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(rContext->commandBuffers[i], indexCount, 1, 0, 0, 0);

    vkCmdEndRenderPass(rContext->commandBuffers[i]);

    IVK_ASSERT(vkEndCommandBuffer(rContext->commandBuffers[i]), "Failed to record command buffer");
  }
}

void VulkanBackend::Resize(u32 _width /*= 0*/, u32 _height /*= 0*/)
{
  shouldResize = true;
}

//=================================================================================================
// Rendering component management
//=================================================================================================

void VulkanBackend::InitializeComponents()
{
  CreateComponents();

  CreateDescriptorPool();
  CreateSyncObjects();
  CreateCommandBuffers();
}

void VulkanBackend::CreateComponents()
{
  CreateSwapchain();
  CreateRenderpass();
  CreateDepthImage();

  CreateFramebuffers();
}

void VulkanBackend::DestroyComponents()
{
  // Destroy framebuffers
  for (const auto& fb : rContext->frameBuffers)
  {
    vkDestroyFramebuffer(rContext->device, fb, rContext->allocator);
  }

  // Destroy depth image
  vkDestroyImageView(rContext->device, rContext->depthImage->view, rContext->allocator);
  vkDestroyImage(rContext->device, rContext->depthImage->image, rContext->allocator);
  vkFreeMemory(rContext->device, rContext->depthImage->memory, rContext->allocator);
  rContext->depthImage->image = VK_NULL_HANDLE;

  // Destroy renderpass
  vkDestroyRenderPass(rContext->device, rContext->renderPass, rContext->allocator);
  // Destroy swapchain
  for (const auto& view : rContext->swapchainImageViews)
  {
    vkDestroyImageView(rContext->device, view, rContext->allocator);
  }

  // Implicitly destroys swapchainImages
  vkDestroySwapchainKHR(rContext->device, rContext->swapchain, rContext->allocator);
}

void VulkanBackend::RecreateComponents()
{
  // Wait for all frames to complete
  vkDeviceWaitIdle(rContext->device);

  DestroyComponents();
  for (IceMaterial* mat : materials)
  {
    ((IvkMaterial*)mat)->DestroyFragileComponents(rContext);
  }
  // Refresh device information
  FillPhysicalDeviceInformation();
  CreateComponents();
  for (IceMaterial* mat : materials)
  {
    ((IvkMaterial*)mat)->CreateFragileComponents(rContext);
  }

  RecordCommandBuffers((IvkMaterial*)materials[0]);
}

//=================================================================================================
// Static rendering components
//=================================================================================================

void VulkanBackend::InitializeAPI()
{
  std::vector<const char*> extensions;
  GetRequiredPlatformExtensions(extensions);
  extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
  #ifdef ICE_DEBUG_ONLY
  extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  #endif

  std::vector<const char*> layers;
  #ifdef ICE_DEBUG_ONLY
  layers.push_back("VK_LAYER_KHRONOS_validation");
  #endif

  // Basic application metadata
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.apiVersion = VK_API_VERSION_1_2;
  appInfo.pEngineName = "Ice";
  appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
  // TODO : Make this editable from the client
  appInfo.pApplicationName = "Ice Application";
  appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);

  // Create the vkInstance
  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;
  createInfo.enabledExtensionCount = (u32)extensions.size();
  createInfo.ppEnabledExtensionNames = extensions.data();
  createInfo.enabledLayerCount = (u32)layers.size();
  createInfo.ppEnabledLayerNames = layers.data();

  IVK_ASSERT(vkCreateInstance(&createInfo, rContext->allocator, &rContext->instance),
    "Failed to create instance");
}

void VulkanBackend::CreateLogicalDevice()
{
  LogInfo("Graphics : %u\nPresent : %u\nTransfer : %u\n",
    rContext->graphicsIdx, rContext->presentIdx, rContext->transferIdx);

  VkPhysicalDeviceFeatures enabledFeatures{};
  enabledFeatures.samplerAnisotropy = VK_TRUE;

  u32 queueCount = 3;
  u32 queueIndices[3] = { rContext->graphicsIdx, rContext->presentIdx, rContext->transferIdx };
  const float priority = 1.0f;
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(queueCount);
  for (u32 i = 0; i < queueCount; i++)
  {
    queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[i].queueFamilyIndex = queueIndices[i];
    queueCreateInfos[i].queueCount = 1;
    queueCreateInfos[i].pQueuePriorities = &priority;
  }

  std::vector<const char*> extensions;
  extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

  std::vector<const char*> layers;
  #ifdef ICE_DEBUG_ONLY
  layers.push_back("VK_LAYER_KHRONOS_validation");
  #endif // ICE_DEBUG_ONLY


  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pEnabledFeatures = &enabledFeatures;
  createInfo.enabledExtensionCount = (u32)extensions.size();
  createInfo.ppEnabledExtensionNames = extensions.data();
  createInfo.enabledLayerCount = (u32)layers.size();
  createInfo.ppEnabledLayerNames = layers.data();
  createInfo.queueCreateInfoCount = queueCount;
  createInfo.pQueueCreateInfos = queueCreateInfos.data();

  IVK_ASSERT(vkCreateDevice(rContext->gpu.device, &createInfo,
                            rContext->allocator, &rContext->device),
    "Failed to create vkDevice");

  vkGetDeviceQueue(rContext->device, rContext->graphicsIdx, 0, &rContext->graphicsQueue);
  vkGetDeviceQueue(rContext->device, rContext->presentIdx, 0, &rContext->presentQueue);
  vkGetDeviceQueue(rContext->device, rContext->transferIdx, 0, &rContext->transferQueue);
}

void VulkanBackend::CreateDescriptorPool()
{
  VkDescriptorPoolSize poolSizes[2] = {};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = 3;
  poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[1].descriptorCount = 9;

  VkDescriptorPoolCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  createInfo.poolSizeCount = 2;
  createInfo.pPoolSizes = poolSizes;
  createInfo.maxSets = 3;

  IVK_ASSERT(vkCreateDescriptorPool(rContext->device, &createInfo,
                                    rContext->allocator, &rContext->descriptorPool),
    "Failed to create descriptor pool");
}

void VulkanBackend::CreateSyncObjects()
{
  rContext->syncObjects.imageAvailableSemaphores.resize(MAX_FLIGHT_IMAGE_COUNT);
  rContext->syncObjects.renderCompleteSemaphores.resize(MAX_FLIGHT_IMAGE_COUNT);
  rContext->syncObjects.flightFences.resize(MAX_FLIGHT_IMAGE_COUNT);
  rContext->syncObjects.imageIsInFlightFences.resize(
      rContext->swapchainImages.size(), VK_NULL_HANDLE);

  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (u32 i = 0; i < MAX_FLIGHT_IMAGE_COUNT; i++)
  {
    IVK_ASSERT(vkCreateFence(rContext->device, &fenceInfo, rContext->allocator,
                             &rContext->syncObjects.flightFences[i]),
      "Failed to create fence");
    IVK_ASSERT(vkCreateSemaphore(rContext->device, &semaphoreInfo, rContext->allocator,
      &rContext->syncObjects.imageAvailableSemaphores[i]), "Failed to create image semaphore");
    IVK_ASSERT(vkCreateSemaphore(rContext->device, &semaphoreInfo, rContext->allocator,
      &rContext->syncObjects.renderCompleteSemaphores[i]), "Failed to create render semaphore");
  }
}

void VulkanBackend::CreateCommandPool(VkCommandPool& _pool, u32 _queueIndex,
  VkCommandPoolCreateFlags _flags /*= 0*/)
{
  VkCommandPoolCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  createInfo.queueFamilyIndex = _queueIndex;
  createInfo.flags = _flags;
  IVK_ASSERT(vkCreateCommandPool(rContext->device, &createInfo, rContext->allocator, &_pool),
    "Failed to create command pool for queue family %u", _queueIndex);
}

//=================================================================================================
// Rendering components
//=================================================================================================

void VulkanBackend::CreateSwapchain()
{
  // Find the best format
  VkSurfaceFormatKHR formatInfo = rContext->gpu.surfaceFormats[0];
  for (const auto& sFormat : rContext->gpu.surfaceFormats)
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
  for (const auto& pMode : rContext->gpu.presentModes)
  {
    if (pMode == VK_PRESENT_MODE_MAILBOX_KHR)
    {
      presentMode = pMode;
      break;
    }
  }

  // Get the device's extent
  VkSurfaceCapabilitiesKHR& capabilities = rContext->gpu.surfaceCapabilities;
  VkExtent2D extent;
  if (capabilities.currentExtent.width != UINT32_MAX)
  {
    extent = capabilities.currentExtent;
  }
  else
  {
    extent = GetWindowExtent();

    // TODO : Replace with clamp function calls
    if (extent.width < capabilities.minImageExtent.width)
    {
      extent.width = capabilities.minImageExtent.width;
    }
    else if (extent.width > capabilities.maxImageExtent.width)
    {
      extent.width = capabilities.maxImageExtent.width;
    }

    if (extent.height < capabilities.minImageExtent.height)
    {
      extent.height = capabilities.minImageExtent.height;
    }
    else if (extent.height > capabilities.maxImageExtent.height)
    {
      extent.height = capabilities.maxImageExtent.height;
    }
  }

  LogInfo("Width: %u -- Height: %u", extent.width, extent.height);

  // Choose an image count
  u32 imageCount = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0 && capabilities.maxImageCount < imageCount)
  {
    imageCount = capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = rContext->surface;
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
  if (rContext->graphicsIdx != rContext->presentIdx)
  {
    u32 sharedIndices[] = {rContext->graphicsIdx, rContext->presentIdx};
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = sharedIndices;
  }
  else
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  IVK_ASSERT(vkCreateSwapchainKHR(rContext->device, &createInfo,
                                  rContext->allocator, &rContext->swapchain),
             "Failed to create swapchain");

  rContext->swapchainFormat = formatInfo.format;
  rContext->renderExtent = extent;

  vkGetSwapchainImagesKHR(rContext->device, rContext->swapchain, &imageCount, nullptr);
  rContext->swapchainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(rContext->device, rContext->swapchain, &imageCount,
                          rContext->swapchainImages.data());

  rContext->swapchainImageViews.resize(imageCount);
  for (u32 i = 0; i < imageCount; i++)
  {
    rContext->swapchainImageViews[i] =
        CreateImageView(rContext->swapchainFormat, VK_IMAGE_ASPECT_COLOR_BIT,
                        rContext->swapchainImages[i]);

    if (rContext->swapchainImageViews[i] == VK_NULL_HANDLE)
    {
      LogInfo("Failed to create swapchain image view");
    }
  }
}

void VulkanBackend::CreateRenderpass()
{
  VkAttachmentDescription colorDesc {};
  colorDesc.format = rContext->swapchainFormat;
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

  IVK_ASSERT(vkCreateRenderPass(rContext->device, &creteInfo,
                                rContext->allocator, &rContext->renderPass),
             "Failed to create renderpass");
}

void VulkanBackend::CreateDepthImage()
{
  VkFormat format = FindDepthFormat();
  u32 imageIdx = CreateImage(
      rContext->renderExtent.width, rContext->renderExtent.height, format, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  rContext->depthImage = iceImages[imageIdx];
  rContext->depthImage->view = CreateImageView(format, VK_IMAGE_ASPECT_DEPTH_BIT,
                                              rContext->depthImage->image);
}

void VulkanBackend::CreateFramebuffers()
{
  VkFramebufferCreateInfo createInfo {};
  createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  createInfo.renderPass = rContext->renderPass;
  createInfo.layers = 1;
  createInfo.width = rContext->renderExtent.width;
  createInfo.height = rContext->renderExtent.height;

  u32 imageCount = static_cast<u32>(rContext->swapchainImages.size());
  rContext->frameBuffers.resize(imageCount);

  for (u32 i = 0; i < imageCount; i++)
  {
    VkImageView attachments[] = {rContext->swapchainImageViews[i], rContext->depthImage->view};
    createInfo.attachmentCount = 2;
    createInfo.pAttachments = attachments;

    IVK_ASSERT(vkCreateFramebuffer(rContext->device, &createInfo,
                                   rContext->allocator, &rContext->frameBuffers[i]),
               "Failed to create framebuffers");
  }
}

void VulkanBackend::CreateCommandBuffers()
{
  u32 bufferCount = static_cast<u32>(rContext->swapchainImages.size());
  rContext->commandBuffers.resize(bufferCount);

  VkCommandBufferAllocateInfo allocInfo {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = bufferCount;
  allocInfo.commandPool = rContext->graphicsCommandPool;

  IVK_ASSERT(vkAllocateCommandBuffers(rContext->device, &allocInfo, rContext->commandBuffers.data()),
             "Failed to allocate command buffers");
}

//=================================================================================================
// GPU
//=================================================================================================

void VulkanBackend::ChoosePhysicalDevice()
{
  std::vector<const char*> extensions;
  extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

  // Get all available physical devices
  u32 deviceCount;
  vkEnumeratePhysicalDevices(rContext->instance, &deviceCount, nullptr);
  std::vector<VkPhysicalDevice> physDevices(deviceCount);
  vkEnumeratePhysicalDevices(rContext->instance, &deviceCount, physDevices.data());

  u32 propertyCount;

  struct BestGPU
  {
    VkPhysicalDevice device;
    u32 graphicsIndex;
    u32 presentIndex;
    u32 transferIndex;
  };
  BestGPU bestFit{};

  u32 graphicsIdx, presentIdx, transferIdx;

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

    std::set<std::string> requiredExtensionSet(extensions.begin(), extensions.end());

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(pdevice, &features);

    for (const auto& ext : physicalDeviceExt)
    {
      requiredExtensionSet.erase(ext.extensionName);
    }

    graphicsIdx = GetQueueIndex(queueProperties, VK_QUEUE_GRAPHICS_BIT);
    transferIdx = GetQueueIndex(queueProperties, VK_QUEUE_TRANSFER_BIT);
    presentIdx = GetPresentIndex(&pdevice, propertyCount, graphicsIdx);

    if (
      features.samplerAnisotropy &&
      requiredExtensionSet.empty() &&
      graphicsIdx != -1 &&
      presentIdx != -1 &&
      transferIdx != -1)
    {
      if (graphicsIdx == presentIdx ||
        graphicsIdx == transferIdx ||
        presentIdx == transferIdx)
      {
        bestFit.device = pdevice;
        bestFit.graphicsIndex = graphicsIdx;
        bestFit.presentIndex = presentIdx;
        bestFit.transferIndex = transferIdx;
      }
      else
      {
        rContext->gpu.device = pdevice;
        rContext->graphicsIdx = graphicsIdx;
        rContext->presentIdx = presentIdx;
        rContext->transferIdx = transferIdx;
        return;
      }
    }
  }

  if (bestFit.device != VK_NULL_HANDLE)
  {
    rContext->gpu.device = bestFit.device;
    rContext->graphicsIdx = bestFit.graphicsIndex;
    rContext->presentIdx = bestFit.presentIndex;
    rContext->transferIdx = bestFit.transferIndex;
    return;
  }

  LogInfo("Failed to find a suitable GPU");
}

void VulkanBackend::FillPhysicalDeviceInformation()
{
  IcePhysicalDevice& dInfo = rContext->gpu;
  VkPhysicalDevice& pDevice = rContext->gpu.device;

  u32 count = 0;
  vkGetPhysicalDeviceFeatures(pDevice, &dInfo.features);
  vkGetPhysicalDeviceProperties(pDevice, &dInfo.properties);
  vkGetPhysicalDeviceMemoryProperties(pDevice, &dInfo.memProperties);
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pDevice, rContext->surface, &dInfo.surfaceCapabilities);

  vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &count, nullptr);
  dInfo.queueFamilyProperties.resize(count);
  vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &count, dInfo.queueFamilyProperties.data());

  vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, rContext->surface, &count, nullptr);
  dInfo.presentModes.resize(count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      pDevice, rContext->surface, &count, dInfo.presentModes.data());

  vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, rContext->surface, &count, nullptr);
  dInfo.surfaceFormats.resize(count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(
      pDevice, rContext->surface, &count, dInfo.surfaceFormats.data());
}

u32 VulkanBackend::GetQueueIndex(
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

u32 VulkanBackend::GetPresentIndex(
  const VkPhysicalDevice* _device, u32 _queuePropertyCount, u32 _graphicsIndex)
{
  u32 bestfit = -1;
  VkBool32 supported;


  for (u32 i = 0; i < _queuePropertyCount; i++)
  {
    vkGetPhysicalDeviceSurfaceSupportKHR(*_device, i, rContext->surface, &supported);
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

//=================================================================================================
// NON-API HELPERS
//=================================================================================================

mesh_t VulkanBackend::CreateMesh(const char* _directory)
{
  mesh_t m = FileSystem::LoadMesh(_directory);
  vertexBuffer = CreateAndFillBuffer(rContext,
                                     m.vertices.data(),
                                     sizeof(vertex_t) * m.vertices.size(),
                                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  indexBuffer = CreateAndFillBuffer(rContext,
                                    m.indices.data(),
                                    sizeof(u32) * m.indices.size(),
                                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
  indexCount = static_cast<u32>(m.indices.size());
  return m;
}

iceTexture_t* VulkanBackend::GetTexture(std::string _directory)
{
  std::string fullDir("res/textures/");
  fullDir.append(_directory);

  for (u32 i = 0; i < iceTextures.size(); i++)
  {
    if (strcmp(fullDir.c_str(), iceTextures[i]->directory.c_str()) == 0)
      return iceTextures[i];
  }

  // Texture not found
  return CreateTexture(fullDir);
}

iceTexture_t* VulkanBackend::CreateTexture(std::string _directory)
{
  int width, height;
  void* imageFile = FileSystem::LoadImageFile(_directory.c_str(), width, height);
  VkDeviceSize size = 4 * (VkDeviceSize)width * (VkDeviceSize)height;

  IvkBuffer stagingBuffer(rContext,
                          static_cast<u32>(size),
                          VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  FillBuffer(rContext, stagingBuffer.GetMemory(), imageFile, size);

  FileSystem::DestroyImageFile(imageFile);

  u32 imageIdx = CreateImage(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  iceImage_t* image = iceImages[imageIdx];
  TransitionImageLayout(image->image, VK_FORMAT_R8G8B8A8_UNORM,
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  CopyBufferToImage(stagingBuffer.GetBuffer(), image->image, width, height);
  TransitionImageLayout(image->image, VK_FORMAT_R8G8B8A8_UNORM,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  stagingBuffer.FreeBuffer(rContext);

  iceTexture_t* tex = new iceTexture_t(_directory);
  tex->imageIndex = imageIdx;
  image->view = CreateImageView(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, image->image);
  image->sampler = CreateSampler();
  iceTextures.push_back(tex);

  return tex;
}

bool WindowResizeCallback(u16 _eventCode, void* _sender, void* _listener, IceEventData _data)
{
  //LogInfo("Window Resize: (%u, %u)", _data.u32[0], _data.u32[1]);

  //rContext->renderExtent = { _data.u32[0], _data.u32[1] };
  RendererBackend* rb = static_cast<RendererBackend*>(_listener);

  rb->Resize(_data.u32[0], _data.u32[1]);

  return true;
}

//=================================================================================================
// HELPERS
//=================================================================================================

void VulkanBackend::DestroyShaderModule(VkShaderModule& _module)
{
  vkDestroyShaderModule(rContext->device, _module, rContext->allocator);
}

VkFormat VulkanBackend::FindSupportedFormat(
    const std::vector<VkFormat>& _formats, VkImageTiling _tiling, VkFormatFeatureFlags _features)
{
  for (VkFormat format : _formats)
  {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(rContext->gpu.device, format, &props);

    if (_tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & _features) == _features)
    {
      return format;
    }
    else if (_tiling == VK_IMAGE_TILING_OPTIMAL &&
             (props.optimalTilingFeatures & _features) == _features)
    {
      return format;
    }
  }

  LogInfo("Failed to find a suitable format");
  return _formats[0];
}

VkFormat VulkanBackend::FindDepthFormat()
{
  return FindSupportedFormat(
    { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
    VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

u32 VulkanBackend::FindMemoryType(u32 _mask, VkMemoryPropertyFlags _flags)
{
  const VkPhysicalDeviceMemoryProperties& props = rContext->gpu.memProperties;

  for (u32 i = 0; i < props.memoryTypeCount; i++)
  {
    if (_mask & (1 << i) && (props.memoryTypes[i].propertyFlags & _flags) == _flags)
    {
      return i;
    }
  }

  LogInfo("Failed to find a suitable memory type");
  return -1;
}

//=================================================================================================
// IMAGES
//=================================================================================================

u32 VulkanBackend::CreateImage(u32 _width, u32 _height, VkFormat _format,
  VkImageTiling _tiling, VkImageUsageFlags _usage, VkMemoryPropertyFlags _memProps)
{
  VkImageCreateInfo createInfo{};
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
  if (vkCreateImage(rContext->device, &createInfo, rContext->allocator, &vImage) != VK_SUCCESS)
  {
    LogInfo("Failed to create vkImages");
    return -1;
  }

  // Allocate device memory for image
  VkMemoryRequirements memReqs;
  vkGetImageMemoryRequirements(rContext->device, vImage, &memReqs);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memReqs.size;
  allocInfo.memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, _memProps);

  VkDeviceMemory vMemory;
  if (vkAllocateMemory(rContext->device, &allocInfo, rContext->allocator, &vMemory) != VK_SUCCESS)
  {
    LogInfo("Failed to allocate texture memory");
  }

  // Bind image and device memory
  vkBindImageMemory(rContext->device, vImage, vMemory, 0);

  iceImage_t* image = new iceImage_t();
  image->image = vImage;
  image->memory = vMemory;
  image->format = _format;
  image->layout = VK_IMAGE_LAYOUT_UNDEFINED;
  iceImages.push_back(image);

  return static_cast<u32>(iceImages.size() - 1);
}

VkImageView VulkanBackend::CreateImageView(
    const VkFormat _format, VkImageAspectFlags _aspect, const VkImage& _image)
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
  if (vkCreateImageView(rContext->device, &createInfo, rContext->allocator, &createdView)
      != VK_SUCCESS)
  {
    LogInfo("Failed to create image view");
    return VK_NULL_HANDLE;
  }

  return createdView;
}

VkSampler VulkanBackend::CreateSampler()
{
  VkSamplerCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  createInfo.magFilter = VK_FILTER_LINEAR;
  createInfo.minFilter = VK_FILTER_LINEAR;
  createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

  createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  createInfo.unnormalizedCoordinates = VK_FALSE;
  createInfo.compareEnable = VK_FALSE;
  createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  createInfo.mipLodBias = 0.0f;
  createInfo.minLod = 0.0f;
  createInfo.maxLod = 0.0f;

  createInfo.anisotropyEnable = VK_TRUE;
  createInfo.maxAnisotropy = rContext->gpu.properties.limits.maxSamplerAnisotropy;

  VkSampler sampler;
  IVK_ASSERT(vkCreateSampler(rContext->device, &createInfo, rContext->allocator, &sampler),
    "Failed to create texture sampler");

  return sampler;
}

void VulkanBackend::TransitionImageLayout(
  VkImage _image, VkFormat _format, VkImageLayout _oldLayout, VkImageLayout _newLayout,
  VkPipelineStageFlagBits _shaderStage /*= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT*/)
{
  VkCommandBuffer command = rContext->BeginSingleTimeCommand(rContext->graphicsCommandPool);

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = _oldLayout;
  barrier.newLayout = _newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = _image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;

  VkPipelineStageFlagBits srcStage = VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM;
  VkPipelineStageFlagBits dstStage = VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM;

  if (_oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
      _newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
  {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else if (_oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
           _newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
  {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dstStage = _shaderStage;
  }
  else
  {
    LogInfo("Encountered an unhandled image layout transition");
  }

  vkCmdPipelineBarrier(command, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
  rContext->EndSingleTimeCommand(command, rContext->graphicsCommandPool, rContext->graphicsQueue);
}

void VulkanBackend::CopyBufferToImage(VkBuffer _buffer, VkImage _iamge, u32 _width, u32 _height)
{
  VkCommandBuffer command = rContext->BeginSingleTimeCommand(rContext->transientCommandPool);

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.layerCount = 1;
  region.imageSubresource.baseArrayLayer = 0;

  region.imageOffset = { 0, 0, 0 };
  region.imageExtent = { _width, _height, 1 };

  vkCmdCopyBufferToImage(
    command, _buffer, _iamge, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
  rContext->EndSingleTimeCommand(command, rContext->transientCommandPool, rContext->transferQueue);
}

//=================================================================================================
// SHADERS
//=================================================================================================

IceShader VulkanBackend::CreateShader(const char* _name, IceShaderStageFlags _stage)
{
  IceShader s(_name, _stage);

  std::string fileDir = ICE_RESOURCE_SHADER_DIR;
  fileDir.append(_name);
  std::string layoutDir(fileDir);

  switch (_stage)
  {
  case Ice_Shader_Vert:
    fileDir.append(".vspv");
    layoutDir.append(".vlayout");
    break;
  case Ice_Shader_Frag:
    fileDir.append(".fspv");
    layoutDir.append(".flayout");
    break;
  case Ice_Shader_Comp:
    fileDir.append(".cspv");
    layoutDir.append(".clayout");
    break;
  }

  // Load shader file
  std::vector<char> shaderCode = FileSystem::LoadFile(fileDir.c_str());
  //std::vector<char> shaderLayout = Platform::LoadFile(layoutDir);

  // Create VkShaderModule
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = shaderCode.size();
  createInfo.pCode = reinterpret_cast<const u32*>(shaderCode.data());

  IVK_ASSERT(vkCreateShaderModule(rContext->device, &createInfo, rContext->allocator, &s.module),
    "Failed to create shader module");
  return s;
}

void VulkanBackend::CreateDescriptorSet(u32 _programIndex)
{
  iceShaderProgram_t& shaderProgram = *GetShaderProgram(_programIndex);

  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = rContext->descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &shaderProgram.descriptorSetLayout;

  IVK_ASSERT(vkAllocateDescriptorSets(rContext->device, &allocInfo, &shaderProgram.descriptorSet),
    "Failed to allocate descriptor set");

  std::vector<VkWriteDescriptorSet> writeSets(shaderProgram.bindings.size());
  std::vector<VkDescriptorImageInfo> imageInfos(shaderProgram.bindings.size() - 1);

  // Create the Uniform buffer
  VkDescriptorBufferInfo mvpBufferInfo{};
  mvpBufferInfo.buffer = mvpBuffer->GetBuffer();
  mvpBufferInfo.offset = 0;
  mvpBufferInfo.range = VK_WHOLE_SIZE;

  //VkWriteDescriptorSet descWrites[2] = {};
  // MVP matrix
  writeSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writeSets[0].dstSet = shaderProgram.descriptorSet;
  writeSets[0].dstBinding = 0;
  writeSets[0].dstArrayElement = 0;
  writeSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  writeSets[0].descriptorCount = 1;
  writeSets[0].pBufferInfo = &mvpBufferInfo;
  writeSets[0].pImageInfo = nullptr;
  writeSets[0].pTexelBufferView = nullptr;

  // Fill info for each texture
  for (u32 i = 1, imageBufferIdx = 0; i < shaderProgram.bindings.size(); i++)
  {
    if (shaderProgram.bindings[i] == Ice_Shader_Binding_Image)
    {
      u32 textureImageIdx = GetTexture(shaderProgram.textureDirs[imageBufferIdx])->imageIndex;
      VkDescriptorImageInfo iInfo{};
      imageInfos[imageBufferIdx].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfos[imageBufferIdx].sampler = iceImages[textureImageIdx]->sampler;
      imageInfos[imageBufferIdx].imageView = iceImages[textureImageIdx]->view;

      // Texture
      writeSets[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      writeSets[i].dstSet = shaderProgram.descriptorSet;
      writeSets[i].dstBinding = i;
      writeSets[i].dstArrayElement = 0;
      writeSets[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      writeSets[i].descriptorCount = 1;
      writeSets[i].pBufferInfo = nullptr;
      writeSets[i].pImageInfo = &imageInfos[imageBufferIdx];
      writeSets[i].pTexelBufferView = nullptr;

      imageBufferIdx++;
    }
  }

  vkUpdateDescriptorSets(rContext->device, (u32)writeSets.size(), writeSets.data(), 0, nullptr);
}
