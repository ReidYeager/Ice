
#include "defines.h"
#include "asserts.h"
#include "logger.h"

#include "rendering/vulkan/vk_renderer.h"

#include "core/input.h"
#include "core/camera.h"
#include "libraries/imgui/imgui.h"
#include "platform/platform.h"
#include "platform/file_system.h"
#include "rendering/mesh.h"

#include <vector>

b8 IvkRenderer::Initialize(const IceRendererSettings& _settings)
{
  LOG_DEBUG("===== Vulkan Renderer Init =====");

  // API initialization =====
  ICE_ATTEMPT_BOOL(CreateInstance());
  ICE_ATTEMPT_BOOL(CreateSurface(_settings.window));
  ICE_ATTEMPT_BOOL(ChoosePhysicalDevice());
  ICE_ATTEMPT_BOOL(CreateLogicalDevice());

  // Rendering components =====
  ICE_ATTEMPT_BOOL(CreateDescriptorPool());
  ICE_ATTEMPT_BOOL(CreateCommandPool());
  ICE_ATTEMPT_BOOL(CreateCommandPool(true));

  ICE_ATTEMPT_BOOL(CreateSwapchain());

  // Others =====
  ICE_ATTEMPT_BOOL(CreateSyncObjects());
  ICE_ATTEMPT_BOOL(CreateCommandBuffers());

  LOG_DEBUG("===== Vulkan Renderer Init Complete =====");

  context.defaultColorImage = GetTexture("TestAlbedo.png", Ice_Image_Color);
  context.defaultNormalImage = GetTexture("EmptyNormal.png", Ice_Image_Normal);
  context.defaultDepthMapImage = GetTexture("PixelWhite.png", Ice_Image_Depth);

  // Remove from initialization? =====
  {
    // Deferred =====
    ICE_ATTEMPT_BOOL(CreateDeferredRenderpass());
    ICE_ATTEMPT_BOOL(CreateDeferredFramebuffers());

    // Shadows =====
    ICE_ATTEMPT_BOOL(CreateShadowRenderpass());
    ICE_ATTEMPT_BOOL(CreateShadowFrameBuffer());

    // Forward =====
    ICE_ATTEMPT_BOOL(CreateForwardRenderpass());
    ICE_ATTEMPT_BOOL(CreateForwardFramebuffers());

    // Descriptors =====
    ICE_ATTEMPT_BOOL(PrepareGlobalDescriptors());
    ICE_ATTEMPT_BOOL(PrepareShadowDescriptors());

    // GUI =====
    //InitImgui();
  }

  return true;
}

b8 IvkRenderer::Shutdown()
{
  vkDeviceWaitIdle(context.device);

  // Shadow =====
  DestroyBuffer(&shadow.lightMatrixBuffer, true);
  DestroyImage(&shadow.image);
  vkDestroyRenderPass(context.device, shadow.renderpass, context.alloc);
  //vkFreeDescriptorSets(context.device, context.descriptorPool, 1, &shadow.descriptorSet);
  vkDestroyFramebuffer(context.device, shadow.framebuffer, context.alloc);

  // Material =====
  for (auto& s : shaders)
  {
    vkDestroyShaderModule(context.device, s.module, context.alloc);
  }

  for (const auto& mat : materials)
  {
    vkDestroyPipeline(context.device, mat.shadowPipeline, context.alloc);
    vkDestroyPipeline(context.device, mat.pipeline, context.alloc);
    vkDestroyPipelineLayout(context.device, mat.pipelineLayout, context.alloc);
    vkDestroyDescriptorSetLayout(context.device, mat.descriptorSetLayout, context.alloc);
  }

  for (auto& b : materialBuffers)
  {
    vkDestroyBuffer(context.device, b.buffer, context.alloc);
    vkFreeMemory(context.device, b.memory, context.alloc);
  }

  for (auto& t : textures)
  {
    DestroyImage(&t.image);
  }

  for (u32 i = 0; i < scene.size(); i++)
  {
    for (const auto& obj : scene[i])
    {
      //vkFreeDescriptorSets(context.device, context.descriptorPool, 1, &obj.descriptorSet);
      DestroyBuffer(&obj.transformBuffer, true);
    }
  }
  vkDestroyDescriptorSetLayout(context.device, context.objectDescriptorSetLayout, context.alloc);

  for (const auto& mesh : meshes)
  {
    DestroyBuffer(&mesh.vertBuffer);
    DestroyBuffer(&mesh.indexBuffer);
  }

  // Global descriptors =====
  DestroyBuffer(&globalDescriptorBuffer);
  vkDestroyPipelineLayout(context.device, context.globalPipelinelayout, context.alloc);
  vkDestroyDescriptorSetLayout(context.device, context.globalDescriptorSetLayout, context.alloc);

  // Rendering components =====
  vkFreeCommandBuffers(context.device,
                       context.graphicsCommandPool,
                       context.commandBuffers.size(),
                       context.commandBuffers.data());

  for (u32 i = 0; i < ICE_MAX_FLIGHT_IMAGE_COUNT; i++)
  {
    vkDestroyFence(context.device, context.flightSlotAvailableFences[i], context.alloc);
    vkDestroySemaphore(context.device, context.renderCompleteSemaphores[i], context.alloc);
    vkDestroySemaphore(context.device, context.imageAvailableSemaphores[i], context.alloc);
  }

  // Forward rendering =====
  vkDestroyRenderPass(context.device, context.forwardRenderpass, context.alloc);
  for (const auto& f : context.forwardFrameBuffers)
  {
    vkDestroyFramebuffer(context.device, f, context.alloc);
  }

  // Deferred rendering =====
  DestroyImage(&context.geoBuffer.position);
  DestroyImage(&context.geoBuffer.normal);
  DestroyImage(&context.geoBuffer.albedo);
  DestroyImage(&context.geoBuffer.maps);
  DestroyImage(&context.geoBuffer.depth);
  for (const auto& f : context.deferredFramebuffers)
  {
    vkDestroyFramebuffer(context.device, f, context.alloc);
  }
  vkDestroyRenderPass(context.device, context.deferredRenderpass, context.alloc);

  // Swapchain =====
  for (const auto& v : context.swapchainImageViews)
  {
    vkDestroyImageView(context.device, v, context.alloc);
  }
  vkDestroySwapchainKHR(context.device, context.swapchain, context.alloc);

  // GUI =====
  //ShutdownImgui();

  // Command pool =====
  vkDestroyCommandPool(context.device, context.graphicsCommandPool, context.alloc);
  vkDestroyCommandPool(context.device, context.transientCommandPool, context.alloc);
  vkDestroyDescriptorPool(context.device, context.descriptorPool, context.alloc);

  // API shutdown =====
  vkDestroyDevice(context.device, context.alloc);
  vkDestroySurfaceKHR(context.instance, context.surface, context.alloc);
  vkDestroyInstance(context.instance, context.alloc);
  return true;
}

b8 IvkRenderer::Render(IceCamera* _camera)
{
  // TODO : Move out of IvkRenderer
  {
    //ImGui::Begin("Directional light");
    //ImGui::DragFloat3("Light color", &tmpLights.directional.color.x, 0.01f, 0.0f, 1.0f);
    //ImGui::DragFloat("Light intensity", &tmpLights.directional.color.w, 0.1f);
    //ImGui::SliderFloat("direction x", &tmpLights.directional.direction.x, -1.0f, 1.0f);
    //ImGui::SliderFloat("direction y", &tmpLights.directional.direction.y, -1.0f, 1.0f);
    //ImGui::SliderFloat("direction z", &tmpLights.directional.direction.z, -1.0f, 1.0f);

    //// normalize direction
    //f32 xsq = tmpLights.directional.direction.x * tmpLights.directional.direction.x;
    //f32 ysq = tmpLights.directional.direction.y * tmpLights.directional.direction.y;
    //f32 zsq = tmpLights.directional.direction.z * tmpLights.directional.direction.z;
    //f32 sq = xsq + ysq + zsq;
    //sq = sqrt(sq);
    //tmpLights.directional.direction.x /= sq;
    //tmpLights.directional.direction.y /= sq;
    //tmpLights.directional.direction.z /= sq;

    //ImGui::End();

    // Update buffer data =====
    // Directional light matrix =====
    float size = 10.0f;
    glm::mat4 proj = glm::ortho(-size, size, -size, size, -100.0f, 100.0f);
    proj[1][1] *= -1;
    glm::mat4 view = glm::lookAt(glm::vec3(tmpLights.directional.direction.x,
                                           tmpLights.directional.direction.y,
                                           tmpLights.directional.direction.z) * -10.0f,
                                 glm::vec3(0, 0, 0),
                                 glm::vec3(0, 1, 0));
    shadow.viewProjMatrix = proj * view;

    FillBuffer(&globalDescriptorBuffer, (void*)&shadow.viewProjMatrix, 64, 64 + sizeof(IvkLights));
    FillBuffer(&shadow.lightMatrixBuffer, (void*)&shadow.viewProjMatrix, 64);

    // Camera matrix =====
    FillBuffer(&globalDescriptorBuffer, (void*)&_camera->viewProjectionMatrix, 64);
    FillBuffer(&globalDescriptorBuffer, (void*)&tmpLights, sizeof(IvkLights), 64);
  }


  static u32 flightSlotIndex = 0;
  static u32 swapchainImageIndex = 0;
  VkResult result;

  // Wait for oldest in-flight slot to return =====
  IVK_ASSERT(vkWaitForFences(context.device,
                             1,
                             &context.flightSlotAvailableFences[flightSlotIndex],
                             VK_TRUE,
                             3000000000), // 3 second timeout
             "Flight slot wait fence failed");

  result = vkAcquireNextImageKHR(context.device,
                                 context.swapchain,
                                 UINT64_MAX,
                                 context.imageAvailableSemaphores[flightSlotIndex],
                                 VK_NULL_HANDLE,
                                 &swapchainImageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR)
  {
    Resize();
    return true;
  }
  else if (result != VK_SUCCESS)
  {
    return false;
  }

  // Submit a command buffer =====
  RecordCommandBuffer(swapchainImageIndex);

  VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

  VkSubmitInfo submitInfo { VK_STRUCTURE_TYPE_SUBMIT_INFO };
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &context.imageAvailableSemaphores[flightSlotIndex];
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &context.commandBuffers[swapchainImageIndex];
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &context.renderCompleteSemaphores[flightSlotIndex];

  IVK_ASSERT(vkResetFences(context.device,
                           1,
                           &context.flightSlotAvailableFences[flightSlotIndex]),
             "Failed to reset flight slot available fence %u", flightSlotIndex);
  IVK_ASSERT(vkQueueSubmit(context.graphicsQueue,
                           1,
                           &submitInfo,
                           context.flightSlotAvailableFences[flightSlotIndex]),
             "Failed to submit draw command");

  // Present =====
  VkPresentInfoKHR presentInfo { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &context.swapchain;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &context.renderCompleteSemaphores[flightSlotIndex];
  presentInfo.pImageIndices = &swapchainImageIndex;

  result = vkQueuePresentKHR(context.presentQueue, &presentInfo);

  if (result == VK_ERROR_OUT_OF_DATE_KHR)
  {
    Resize();
  }
  else if (result != VK_SUCCESS)
  {
    IceLogFatal("Failed to present the swapchain");
    return false;
  }

  flightSlotIndex = (flightSlotIndex + 1) % ICE_MAX_FLIGHT_IMAGE_COUNT;

  return true;
}

b8 IvkRenderer::Resize()
{
  vkDeviceWaitIdle(context.device);

  LOG_DEBUG("Resizing");

  // Swapchain =====
  for (auto& v : context.swapchainImageViews)
  {
    vkDestroyImageView(context.device, v, context.alloc);
  }
  context.swapchainImageViews.clear();
  vkDestroySwapchainKHR(context.device, context.swapchain, context.alloc);

  ICE_ATTEMPT_BOOL(CreateSwapchain());

  // Deferred =====
  DestroyImage(&context.geoBuffer.position);
  DestroyImage(&context.geoBuffer.normal);
  DestroyImage(&context.geoBuffer.albedo);
  DestroyImage(&context.geoBuffer.maps);
  DestroyImage(&context.geoBuffer.depth);
  for (auto& f : context.deferredFramebuffers)
  {
    vkDestroyFramebuffer(context.device, f, context.alloc);
  }
  context.deferredFramebuffers.clear();
  vkDestroyRenderPass(context.device, context.deferredRenderpass, context.alloc);

  ICE_ATTEMPT_BOOL(CreateDeferredRenderpass());
  ICE_ATTEMPT_BOOL(CreateDeferredFramebuffers());

  // Forward =====
  for (auto& f : context.forwardFrameBuffers)
  {
    vkDestroyFramebuffer(context.device, f, context.alloc);
  }
  context.forwardFrameBuffers.clear();
  vkDestroyRenderPass(context.device, context.forwardRenderpass, context.alloc);

  ICE_ATTEMPT_BOOL(CreateForwardRenderpass());
  ICE_ATTEMPT_BOOL(CreateForwardFramebuffers());

  // Update descriptors =====
  DestroyBuffer(&globalDescriptorBuffer);
  vkDestroyPipelineLayout(context.device, context.globalPipelinelayout, context.alloc);
  vkDestroyDescriptorSetLayout(context.device, context.globalDescriptorSetLayout, context.alloc);
  vkDestroyDescriptorSetLayout(context.device, context.objectDescriptorSetLayout, context.alloc);

  ICE_ATTEMPT_BOOL(PrepareGlobalDescriptors());

  // Materials =====
  for (auto& m : materials)
  {
    vkDestroyPipeline(context.device, m.pipeline, context.alloc);
    vkDestroyPipeline(context.device, m.shadowPipeline, context.alloc);
    CreatePipeline(m);
  }

  return true;
}

b8 IvkRenderer::CreateInstance()
{
  // Retrieve and validate extensions and layers to enable
  std::vector<const char*> extensions;
  GetRequiredPlatformExtensions(extensions);
  extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
  #ifdef ICE_DEBUG
  extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  #endif

  std::vector<const char*> layers;
  #ifdef ICE_DEBUG
  layers.push_back("VK_LAYER_KHRONOS_validation");
  #endif

  VkApplicationInfo appInfo { VK_STRUCTURE_TYPE_APPLICATION_INFO };
  appInfo.apiVersion = VK_API_VERSION_1_2;
  appInfo.pEngineName = "Ice";
  appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);

  // Define this information elsewhere
  appInfo.pApplicationName = "TMP_APPLICAITON_NAME";
  appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);

  VkInstanceCreateInfo createInfo { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
  createInfo.enabledExtensionCount = extensions.size();
  createInfo.ppEnabledExtensionNames = extensions.data();
  createInfo.enabledLayerCount = layers.size();
  createInfo.ppEnabledLayerNames = layers.data();
  createInfo.pApplicationInfo = &appInfo;
  createInfo.flags = 0;
  // Daisy-chain additional create infos here with 'createInfo.pNext'

  IVK_ASSERT(vkCreateInstance(&createInfo, context.alloc, &context.instance),
             "Failed to create vulkan instance");

  return context.instance != VK_NULL_HANDLE;
}

b8 IvkRenderer::ChoosePhysicalDevice()
{
  u32 deviceCount = 0;
  vkEnumeratePhysicalDevices(context.instance, &deviceCount, nullptr);
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(context.instance, &deviceCount, devices.data());

  for (const auto& gpu : devices)
  {
    if (IsDeviceSuitable(gpu))
    {
      context.gpu.device = gpu;
      break;
    }
  }

  if (context.gpu.device == VK_NULL_HANDLE)
    return false;

  // Fill GPU information =====
  IvkGpu& gpu = context.gpu;

  vkGetPhysicalDeviceProperties(gpu.device, &gpu.properties);
  vkGetPhysicalDeviceFeatures(gpu.device, &gpu.features);
  vkGetPhysicalDeviceMemoryProperties(gpu.device, &gpu.memoryProperties);
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu.device, context.surface, &gpu.surfaceCapabilities);

  u32 count;

  vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.device, context.surface, &count, nullptr);
  gpu.surfaceFormats.resize(count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.device,
                                       context.surface,
                                       &count,
                                       gpu.surfaceFormats.data());

  vkGetPhysicalDeviceQueueFamilyProperties(gpu.device, &count, nullptr);
  gpu.queueFamilyProperties.resize(count);
  vkGetPhysicalDeviceQueueFamilyProperties(gpu.device, &count, gpu.queueFamilyProperties.data());

  vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.device, context.surface, &count, nullptr);
  gpu.presentModes.resize(count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.device,
                                            context.surface,
                                            &count,
                                            gpu.presentModes.data());

  // Select queue families
  gpu.graphicsQueueIndex = GetIndexForQueue(VK_QUEUE_GRAPHICS_BIT);
  gpu.transientQueueIndex = GetIndexForQueue(VK_QUEUE_TRANSFER_BIT);
  gpu.presentQueueIndex = GetPresentQueue();

  if (gpu.graphicsQueueIndex == -1u ||
      gpu.presentQueueIndex == -1u  ||
      gpu.transientQueueIndex == -1u)
  {
    IceLogError("Some of the queue indices are invalid (G : %u, P : %u, T : %u)",
                gpu.graphicsQueueIndex,
                gpu.presentQueueIndex,
                gpu.transientQueueIndex);
    return false;
  }

  return true;
}

u32 IvkRenderer::GetIndexForQueue(VkQueueFlags _flags)
{
  u32 bestFit = -1u;

  for (u32 i = 0; i < context.gpu.queueFamilyProperties.size(); i++)
  {
    u32 queueFlag = context.gpu.queueFamilyProperties[i].queueFlags;

    if ((queueFlag & _flags) == _flags)
    {
      // Attempt to avoid queues that share with graphics
      if (_flags & VK_QUEUE_GRAPHICS_BIT || i != context.gpu.graphicsQueueIndex)
      {
        return i;
      }
      else
      {
        bestFit = i;
      }
    }
  }

  if (bestFit == -1u)
  {
    IceLogError("Failed to find a device queue that matches the input %u", _flags);
  }
  return bestFit;
}

u32 IvkRenderer::GetPresentQueue()
{
  VkBool32 canPresent = false;
  u32 bestFit = -1u;

  for (u32 queueIndex = 0; queueIndex < context.gpu.queueFamilyProperties.size(); queueIndex++)
  {
    vkGetPhysicalDeviceSurfaceSupportKHR(context.gpu.device,
                                         queueIndex,
                                         context.surface,
                                         &canPresent);
    if (canPresent)
    {
      // Attempt to avoid queues that share with graphics
      if (queueIndex == context.gpu.graphicsQueueIndex)
      {
        bestFit = queueIndex;
      }
      else
      {
        return queueIndex;
      }
    }
  }

  if (bestFit == -1u)
  {
    IceLogError("Failed to find a device queue that supports presentation");
  }
  return bestFit;
}

b8 IvkRenderer::IsDeviceSuitable(const VkPhysicalDevice& _device)
{
  u32 queueCount;
  vkGetPhysicalDeviceQueueFamilyProperties(_device, &queueCount, nullptr);

  VkBool32 canPresent = false;
  for (u32 queueIndex = 0; queueIndex < queueCount; queueIndex++)
  {
    vkGetPhysicalDeviceSurfaceSupportKHR(_device, queueIndex, context.surface, &canPresent);
    if (canPresent)
    {
      break;
    }
  }

  b8 isSuitable = true;
  isSuitable &= canPresent == VK_TRUE;

  return isSuitable;
}

b8 IvkRenderer::CreateLogicalDevice()
{
  // Use to enable features for use in rendering
  VkPhysicalDeviceFeatures enabledFeatures{};

  // Queues =====
  // Graphics, Presentation, Transfer
  const u32 queueCount = 3;
  const float queuePriority = 1.0f;

  u32 queueIndices[queueCount] = { context.gpu.graphicsQueueIndex,
                                   context.gpu.presentQueueIndex,
                                   context.gpu.transientQueueIndex };

  ICE_LOG_INFO("%s -- Graphics : %u -- Presentation : %u -- Transfer : %u",
             context.gpu.properties.deviceName, queueIndices[0], queueIndices[1], queueIndices[2]);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(queueCount);
  for (u32 i = 0; i < queueCount; i++)
  {
    queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[i].flags = 0;
    queueCreateInfos[i].queueFamilyIndex = queueIndices[i];
    queueCreateInfos[i].pQueuePriorities = &queuePriority;
    queueCreateInfos[i].queueCount = 1;
  }

  // Extensions & Layers =====
  std::vector<const char*> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

  std::vector<const char*> layers;
  #ifdef ICE_DEBUG
  layers.push_back("VK_LAYER_KHRONOS_validation");
  #endif // ICE_DEBUG

  // Creation =====
  VkDeviceCreateInfo createInfo { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
  createInfo.flags = 0;
  createInfo.pEnabledFeatures = &enabledFeatures;
  createInfo.queueCreateInfoCount = queueCreateInfos.size();
  createInfo.pQueueCreateInfos    = queueCreateInfos.data();
  createInfo.enabledExtensionCount   = extensions.size();
  createInfo.ppEnabledExtensionNames = extensions.data();
  createInfo.enabledLayerCount   = layers.size();
  createInfo.ppEnabledLayerNames = layers.data();

  IVK_ASSERT(vkCreateDevice(context.gpu.device, &createInfo, nullptr, &context.device),
             "Failed to create Vulkan logical device");

  vkGetDeviceQueue(context.device, context.gpu.graphicsQueueIndex, 0, &context.graphicsQueue);
  vkGetDeviceQueue(context.device, context.gpu.presentQueueIndex , 0, &context.presentQueue );
  vkGetDeviceQueue(context.device, context.gpu.transientQueueIndex, 0, &context.transientQueue);

  return context.device != VK_NULL_HANDLE;
}

b8 IvkRenderer::CreateCommandPool(b8 _createTransient /*= false*/)
{
  VkCommandPool* poolPtr = 0;
  VkCommandPoolCreateInfo createInfo { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };

  if (_createTransient)
  { // Transfer =====
    createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    createInfo.queueFamilyIndex = context.gpu.transientQueueIndex;
    poolPtr = &context.transientCommandPool;
  }
  else
  { // Graphics =====
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = context.gpu.graphicsQueueIndex;
    poolPtr = &context.graphicsCommandPool;
  }

  IVK_ASSERT(vkCreateCommandPool(context.device, &createInfo, context.alloc, poolPtr),
             "Failed to create command pool");

  return true;
}

b8 IvkRenderer::CreateSwapchain()
{
  u32 imageCount = context.gpu.surfaceCapabilities.minImageCount + 1;

  // Create swapchain =====
  {
    u32 maxImageCount = context.gpu.surfaceCapabilities.maxImageCount;
    if (maxImageCount > 0 && imageCount > maxImageCount)
    {
      imageCount = maxImageCount;
    }

    // Format =====
    VkSurfaceFormatKHR format = context.gpu.surfaceFormats[0]; // Default
    for (const auto& f : context.gpu.surfaceFormats)
    {
      if (f.format == VK_FORMAT_R32G32B32A32_SFLOAT &&
          f.colorSpace == VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT)
      {
        format = f;
        break;
      }
    }
    context.surfaceFormat = format;

    // Present mode =====
    VkPresentModeKHR present = VK_PRESENT_MODE_FIFO_KHR; // Guaranteed
    for (const auto& mode : context.gpu.presentModes)
    {
      if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
      {
        present = mode;
        break;
      }
    }
    //present = VK_PRESENT_MODE_FIFO_KHR; // Un-comment for v-sync
    context.presentMode = present;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context.gpu.device, context.surface, &context.gpu.surfaceCapabilities);

    // Image dimensions =====
    VkExtent2D extent;
    if (context.gpu.surfaceCapabilities.currentExtent.width != -1u)
    {
      extent = context.gpu.surfaceCapabilities.currentExtent;
    }
    else
    {
      vec2U e = GetPlatformWindowExtents();
      extent = { e.width, e.height };
    }
    LOG_DEBUG("Swapchain using extents : (%u, %u)", extent.width, extent.height);

    // Creation =====
    VkSwapchainCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    createInfo.flags = 0;
    createInfo.imageArrayLayers = 1;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    createInfo.clipped = VK_TRUE;
    createInfo.imageColorSpace = format.colorSpace;
    createInfo.imageFormat = format.format;
    createInfo.imageExtent = extent;
    createInfo.presentMode = present;
    createInfo.minImageCount = imageCount;
    createInfo.surface = context.surface;

    u32 indices[] = { context.gpu.graphicsQueueIndex, context.gpu.presentQueueIndex };
    if (context.gpu.graphicsQueueIndex != context.gpu.presentQueueIndex)
    {
      createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = indices;
    }
    else
    {
      // NOTE-: Using this, despite having different queues for graphics and presentation, seems to
      // not only work, but improve frame times a bit (~100us for 1280x720 on gtx 1060).
      // I'm leaving in the above because I'm worried that it might later break if removed
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    IVK_ASSERT(vkCreateSwapchainKHR(context.device, &createInfo, context.alloc, &context.swapchain),
               "Failed to create swapchain");

    // Swapchain context =====
    context.swapchainFormat = format.format;
    context.swapchainExtent = extent;
  }

  // Retrieve images and create views =====
  {
    // Get the number of images actually created
    vkGetSwapchainImagesKHR(context.device, context.swapchain, &imageCount, nullptr);

    ICE_LOG_INFO("Created %u images for the swapchain", imageCount);

    context.swapchainImages.resize(imageCount);
    context.swapchainImageViews.resize(imageCount);

    vkGetSwapchainImagesKHR(context.device,
                            context.swapchain,
                            &imageCount,
                            context.swapchainImages.data());

    for (u32 i = 0; i < imageCount; i++)
    {
      ICE_ATTEMPT_BOOL(CreateImageView(&context.swapchainImageViews[i],
                                  context.swapchainImages[i],
                                  context.swapchainFormat,
                                  VK_IMAGE_ASPECT_COLOR_BIT));
    }
  }

  return true;
}

b8 IvkRenderer::CreateSyncObjects()
{
  VkSemaphoreCreateInfo semaphoreInfo { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
  semaphoreInfo.flags = 0;

  VkFenceCreateInfo fenceInfo { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  // Create each frame's sync objects =====
  context.imageAvailableSemaphores.resize(ICE_MAX_FLIGHT_IMAGE_COUNT);
  context.renderCompleteSemaphores.resize(ICE_MAX_FLIGHT_IMAGE_COUNT);
  context.flightSlotAvailableFences.resize(ICE_MAX_FLIGHT_IMAGE_COUNT);

  for (u32 i = 0; i < ICE_MAX_FLIGHT_IMAGE_COUNT; i++)
  {
    IVK_ASSERT(vkCreateFence(context.device,
                             &fenceInfo,
                             context.alloc,
                             &context.flightSlotAvailableFences[i]),
               "Failed to create flight slot fence %u", i);

    IVK_ASSERT(vkCreateSemaphore(context.device,
                                 &semaphoreInfo,
                                 context.alloc,
                                 &context.imageAvailableSemaphores[i]),
               "Failed to create image available semaphore %u", i);

    IVK_ASSERT(vkCreateSemaphore(context.device,
                                 &semaphoreInfo,
                                 context.alloc,
                                 &context.renderCompleteSemaphores[i]),
               "Failed to create render complete semaphore %u", i);
  }

  return true;
}

VkFormat IvkRenderer::GetDepthFormat()
{
  // Find depth format =====
  VkFormat format = VK_FORMAT_D32_SFLOAT;
  VkFormatProperties formatProperties;
  const u32 fCount = 3;
  VkFormat desiredFormats[fCount] = { VK_FORMAT_D32_SFLOAT, // 32-bit [signed float]
                                      VK_FORMAT_D32_SFLOAT_S8_UINT, // 32-bit [signed float], 8-bit [0 to 255]
                                      VK_FORMAT_D24_UNORM_S8_UINT }; // 24-bit [0 to 1], 8-bit [0 to 255]
  for (u32 i = 0; i < fCount; i++)
  {
    vkGetPhysicalDeviceFormatProperties(context.gpu.device, desiredFormats[i], &formatProperties);

    if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
      format = desiredFormats[i];
      break;
    }
  }

  return format;
}

b8 IvkRenderer::SetDeferredLightingMaterial(IceHandle _material)
{
  if (materials.size() < _material || materials[_material].subpassIndex != 1)
  {
    return false;
  }

  context.deferredMaterialIndex = _material;

  return true;
}

//=========================
// V Not vulkan intialization V
//=========================

b8 IvkRenderer::AddMeshToScene(IceObject* _object)
{
  const u32& meshIndex = _object->meshHandle;
  const u32& materialIndex = _object->materialHandle;

  IvkObject object;
  object.mesh = meshes[meshIndex];

  CreateBuffer(&object.transformBuffer,
               64,
               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  _object->transform.buffer = object.transformBuffer;

  VkDescriptorSetAllocateInfo allocInfo { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
  allocInfo.descriptorPool = context.descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &context.objectDescriptorSetLayout;
  allocInfo.pNext = nullptr;

  IVK_ASSERT(vkAllocateDescriptorSets(context.device,
                                      &allocInfo,
                                      &object.descriptorSet),
             "Failed to allocate object descriptor set");

  VkDescriptorBufferInfo bufferInfo {};
  bufferInfo.buffer = object.transformBuffer.buffer;
  bufferInfo.offset = 0;
  bufferInfo.range = VK_WHOLE_SIZE;

  VkWriteDescriptorSet write { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
  write.dstSet           = object.descriptorSet;
  write.dstBinding       = 0;
  write.dstArrayElement  = 0;
  write.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  write.descriptorCount  = 1;
  write.pBufferInfo      = &bufferInfo;
  write.pImageInfo       = nullptr;
  write.pTexelBufferView = nullptr;
  
  vkUpdateDescriptorSets(context.device, 1, &write, 0, nullptr);

  scene[materialIndex].push_back(object);

  return true;
}