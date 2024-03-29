
#include "defines.h"

#include "rendering/vulkan/vulkan.h"
#include "rendering/vulkan/vulkan_defines.h"
#include "core/application.h" // RecreateAllMaterials, ...

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

b8 Ice::RendererVulkan::Init(Ice::RendererSettingsCore _settings, const char* _title, u32 _version)
{
  ICE_ATTEMPT(CreateInstance(_title, _version));
  ICE_ATTEMPT(CreateSurface());
  ICE_ATTEMPT(ChoosePhysicalDevice());
  ICE_ATTEMPT(CreateLogicalDevice());

  ICE_ATTEMPT(CreateDescriptorPool());
  ICE_ATTEMPT(CreateCommandPool());
  ICE_ATTEMPT(CreateCommandPool(true));

  ICE_ATTEMPT(CreateSwapchain());
  ICE_ATTEMPT(CreateSyncObjects());
  ICE_ATTEMPT(CreateCommandBuffers());

  ICE_ATTEMPT(CreateDepthImages());
  ICE_ATTEMPT(CreateGlobalDescriptors());
  ICE_ATTEMPT(CreateForwardComponents());

  Ice::LoadTexture(&defaultTexture, ICE_SOURCE_DIR "/res/defaultTexture.png");

  return true;
}

b8 Ice::RendererVulkan::RenderFrame(Ice::FrameInformation* _data)
{
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
    ICE_ATTEMPT(Resize());
    return true;
  }
  else if (result != VK_SUCCESS)
  {
    return false;
  }

  // Submit a command buffer =====
  RecordCommandBuffer(swapchainImageIndex, _data);

  VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

  VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
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
  VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &context.swapchain;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &context.renderCompleteSemaphores[flightSlotIndex];
  presentInfo.pImageIndices = &swapchainImageIndex;

  // NOTE : Queue present alone takes ~50% of execution time
  result = vkQueuePresentKHR(context.presentQueue, &presentInfo);

  if (result == VK_ERROR_OUT_OF_DATE_KHR)
  {
    ICE_ATTEMPT(Resize());
  }
  else if (result != VK_SUCCESS)
  {
    IceLogFatal("Failed to present the swapchain");
    return false;
  }

  flightSlotIndex = (flightSlotIndex + 1) % ICE_MAX_FLIGHT_IMAGE_COUNT;

  return true;
}

b8 Ice::RendererVulkan::Shutdown()
{
  vkDeviceWaitIdle(context.device);

  vkDestroyDescriptorSetLayout(context.device, context.cameraDescriptorLayout, context.alloc);
  vkDestroyDescriptorSetLayout(context.device, context.objectDescriptorLayout, context.alloc);

  vkDestroyPipelineLayout(context.device, context.globalPipelineLayout, context.alloc);
  vkDestroyDescriptorSetLayout(context.device, context.globalDescriptorLayout, context.alloc);
  DestroyBufferMemory(&context.globalDescriptorBuffer);

  // Renderpasses =====
  for (const auto& f : context.forward.framebuffers)
  {
    vkDestroyFramebuffer(context.device, f, context.alloc);
  }
  context.forward.framebuffers.clear();
  vkDestroyRenderPass(context.device, context.forward.renderpass, context.alloc);

  // Depth =====
  for (auto& d : context.depthImages)
  {
    DestroyImage(&d);
  }
  context.depthImages.clear();

  // Commands =====
  vkFreeCommandBuffers(context.device,
                       context.graphicsCommandPool,
                       (u32)context.commandBuffers.size(),
                       context.commandBuffers.data());
  context.commandBuffers.clear();

  // Synchronization =====
  for (u32 i = 0; i < ICE_MAX_FLIGHT_IMAGE_COUNT; i++)
  {
    vkDestroyFence(context.device, context.flightSlotAvailableFences[i], context.alloc);
    vkDestroySemaphore(context.device, context.renderCompleteSemaphores[i], context.alloc);
    vkDestroySemaphore(context.device, context.imageAvailableSemaphores[i], context.alloc);
  }
  context.flightSlotAvailableFences.clear();
  context.renderCompleteSemaphores.clear();
  context.imageAvailableSemaphores.clear();

  // Swapchain =====
  for (auto& v : context.swapchainImageViews)
  {
    vkDestroyImageView(context.device, v, context.alloc);
  }
  context.swapchainImageViews.clear();
  context.swapchainImages.clear();
  vkDestroySwapchainKHR(context.device, context.swapchain, context.alloc);

  DestroyImage(&defaultTexture);

  // Pools =====
  vkDestroyCommandPool(context.device, context.graphicsCommandPool, context.alloc);
  vkDestroyCommandPool(context.device, context.transientCommandPool, context.alloc);
  vkDestroyDescriptorPool(context.device, context.descriptorPool, context.alloc);

  // Device =====
  vkDestroyDevice(context.device, context.alloc);
  vkDestroySurfaceKHR(context.instance, context.surface, context.alloc);
  vkDestroyInstance(context.instance, context.alloc);

  return true;
}

b8 Ice::RendererVulkan::Resize()
{
  vkDeviceWaitIdle(context.device);

  IceLogDebug(">>> Resizing");

  // Destroy frame components =====
  //Renderpasses
  for (const auto& f : context.forward.framebuffers)
  {
    vkDestroyFramebuffer(context.device, f, context.alloc);
  }
  context.forward.framebuffers.clear();
  vkDestroyRenderPass(context.device, context.forward.renderpass, context.alloc);

  // Swapchain
  for (IvkImage& i : context.depthImages)
  {
    DestroyImage(&i);
  }
  for (VkImageView& v : context.swapchainImageViews)
  {
    vkDestroyImageView(context.device, v, context.alloc);
  }
  context.swapchainImageViews.clear();
  context.swapchainImages.clear();
  vkDestroySwapchainKHR(context.device, context.swapchain, context.alloc);

  vkDestroySurfaceKHR(context.instance, context.surface, context.alloc);

  // Re-create frame components =====
  ICE_ATTEMPT(CreateSurface());
  ICE_ATTEMPT(CreateSwapchain());
  ICE_ATTEMPT(CreateDepthImages());

  ICE_ATTEMPT(CreateForwardComponents());

  // Update materials =====
  // Need to update the viewports in all materials
  ICE_ATTEMPT(RecreateAllMaterials());

  for (Ice::Entity& e : Ice::SceneView<Ice::CameraComponent>())
  {
    Ice::CameraComponent* cc = e.GetComponent<Ice::CameraComponent>();

    cc->settings.ratio = (f32)context.swapchainExtent.width / (f32)context.swapchainExtent.height;
    UpdateCameraProjection(cc, cc->settings);
  }

  IceLogDebug(">>> Complete");

  return true;
}

//=========================
// Vulkan implementation
//=========================

b8 Ice::RendererVulkan::CreateInstance(const char* _title, u32 _version)
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

  VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
  appInfo.apiVersion = VK_API_VERSION_1_2;
  appInfo.pEngineName = "Ice";
  appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);

  appInfo.pApplicationName = _title;
  appInfo.applicationVersion = VK_MAKE_VERSION((_version & 0x00ff0000) >> 16, // Major
                                               (_version & 0x0000ff00) >> 8,  // Minor
                                               (_version & 0x000000ff));      // Patch

  VkInstanceCreateInfo createInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
  createInfo.enabledExtensionCount = (u32)extensions.size();
  createInfo.ppEnabledExtensionNames = extensions.data();
  createInfo.enabledLayerCount = (u32)layers.size();
  createInfo.ppEnabledLayerNames = layers.data();
  createInfo.pApplicationInfo = &appInfo;
  createInfo.flags = 0;
  // Daisy-chain additional create infos here with 'createInfo.pNext'

  IVK_ASSERT(vkCreateInstance(&createInfo, context.alloc, &context.instance),
             "Failed to create vulkan instance");

  return context.instance != VK_NULL_HANDLE;
}

b8 Ice::RendererVulkan::ChoosePhysicalDevice()
{
  u32 deviceCount = 0;
  vkEnumeratePhysicalDevices(context.instance, &deviceCount, nullptr);
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(context.instance, &deviceCount, devices.data());

  int bestRank = 0;
  for (const auto& device : devices)
  {
    int rank = IsDeviceSuitable(device);
    if (rank > bestRank)
    {
      context.gpu.device = device;
      bestRank = rank;
    }
  }

  if (context.gpu.device == VK_NULL_HANDLE)
  {
    IceLogFatal("Failed to select a physical device");
    return false;
  }

  // Fill info =====
  Gpu& gpu = context.gpu;

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

  // Select queue families =====
  context.gpu.graphicsQueueIndex = GetIndexForQueue(VK_QUEUE_GRAPHICS_BIT);
  context.gpu.transientQueueIndex = GetIndexForQueue(VK_QUEUE_TRANSFER_BIT);
  context.gpu.presentQueueIndex = GetPresentQueue();

  if (context.gpu.graphicsQueueIndex == ~0U
      || context.gpu.presentQueueIndex == ~0U
      || context.gpu.transientQueueIndex == ~0U)
  {
    IceLogFatal("Some of the queue indices are invalid (G : %u, P : %u, T : %u)",
                context.gpu.graphicsQueueIndex,
                context.gpu.presentQueueIndex,
                context.gpu.transientQueueIndex);
    return false;
  }

  return true;
}

u32 Ice::RendererVulkan::GetIndexForQueue(VkQueueFlags _flags)
{
  u32 bestFit = ~0U;

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

  if (bestFit == ~0U)
  {
    IceLogError("Failed to find a device queue that matches the input %u", _flags);
  }
  return bestFit;
}

u32 Ice::RendererVulkan::GetPresentQueue()
{
  VkBool32 canPresent = false;
  u32 bestFit = ~0U;

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

  if (bestFit == ~0U)
  {
    IceLogError("Failed to find a device queue that supports presentation");
  }
  return bestFit;
}

u32 Ice::RendererVulkan::IsDeviceSuitable(const VkPhysicalDevice& _device)
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

  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(_device, &properties);

  u32 isSuitable = 0;
  // Support screen presentation
  isSuitable += canPresent == VK_TRUE;
  // Only use dedicated GPUs
  isSuitable += properties.deviceType & VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

  return isSuitable;
}

b8 Ice::RendererVulkan::CreateLogicalDevice()
{
  // Use to enable features for use in rendering
  VkPhysicalDeviceFeatures enabledFeatures{};
  enabledFeatures.fillModeNonSolid = VK_TRUE;

  // Queues =====
  // Graphics, Presentation, Transfer
  const u32 queueCount = 3;
  const float queuePriority = 1.0f;

  u32 queueIndices[queueCount] = { context.gpu.graphicsQueueIndex,
                                   context.gpu.presentQueueIndex,
                                   context.gpu.transientQueueIndex };

  IceLogInfo("%s -- Graphics : %u -- Presentation : %u -- Transfer : %u",
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
  VkDeviceCreateInfo createInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
  createInfo.flags = 0;
  createInfo.pEnabledFeatures = &enabledFeatures;
  createInfo.queueCreateInfoCount = (u32)queueCreateInfos.size();
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.enabledExtensionCount = (u32)extensions.size();
  createInfo.ppEnabledExtensionNames = extensions.data();
  createInfo.enabledLayerCount = (u32)layers.size();
  createInfo.ppEnabledLayerNames = layers.data();

  IVK_ASSERT(vkCreateDevice(context.gpu.device, &createInfo, nullptr, &context.device),
             "Failed to create Vulkan logical device");

  vkGetDeviceQueue(context.device, context.gpu.graphicsQueueIndex, 0, &context.graphicsQueue);
  vkGetDeviceQueue(context.device, context.gpu.presentQueueIndex, 0, &context.presentQueue);
  vkGetDeviceQueue(context.device, context.gpu.transientQueueIndex, 0, &context.transientQueue);

  return context.device != VK_NULL_HANDLE;
}

b8 Ice::RendererVulkan::CreateDescriptorPool()
{
  // Size definitions =====
  const u32 poolSizeCount = 2;
  VkDescriptorPoolSize sizes[poolSizeCount] = {};

  // TODO : Make max uniform descriptor/image descriptor/set counts adjustable
  sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  sizes[0].descriptorCount = 2048; // TMP

  sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  sizes[1].descriptorCount = 2048; // TMP

  // Creation =====
  VkDescriptorPoolCreateInfo createInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
  createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  createInfo.maxSets = 1024; // 0 = Global, 1 = per-camera, 2 = per-material, 3 = per-object
  createInfo.poolSizeCount = poolSizeCount;
  createInfo.pPoolSizes = sizes;

  IVK_ASSERT(vkCreateDescriptorPool(context.device,
                                    &createInfo,
                                    context.alloc,
                                    &context.descriptorPool),
             "Failed to create descriptor pool");

  return true;
}

b8 Ice::RendererVulkan::CreateCommandPool(b8 _createTransient /*= false*/)
{
  VkCommandPool* poolPtr = 0;
  VkCommandPoolCreateInfo createInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };

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

b8 Ice::RendererVulkan::CreateSwapchain()
{
  u32 imageCount = context.gpu.surfaceCapabilities.minImageCount + 1;

  // Create swapchain =====
  {
    // Select best info =====

    u32 maxImageCount = context.gpu.surfaceCapabilities.maxImageCount;
    if (maxImageCount > 0 && imageCount > maxImageCount)
    {
      imageCount = maxImageCount;
    }

    // Format
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

    // Present mode
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

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context.gpu.device,
                                              context.surface,
                                              &context.gpu.surfaceCapabilities);

    // Image dimensions
    VkExtent2D extent;
    if (context.gpu.surfaceCapabilities.currentExtent.width != ~0U)
    {
      extent = context.gpu.surfaceCapabilities.currentExtent;
    }
    else
    {
      Ice::vec2U e = GetWindowExtents();
      extent = { e.width, e.height };
    }
    IceLogInfo("Swapchain using: extents (%u, %u) -- format %d",
               extent.width,
               extent.height,
               format);

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
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    IVK_ASSERT(vkCreateSwapchainKHR(context.device, &createInfo, context.alloc, &context.swapchain),
               "Failed to create swapchain");

    // Swapchain context
    context.swapchainFormat = format.format;
    context.swapchainExtent = extent;
  }

  // Retrieve images and create views =====
  {
    // Get the number of images actually created
    vkGetSwapchainImagesKHR(context.device, context.swapchain, &imageCount, nullptr);

    context.swapchainImages.resize(imageCount);
    context.swapchainImageViews.resize(imageCount);

    vkGetSwapchainImagesKHR(context.device,
                            context.swapchain,
                            &imageCount,
                            context.swapchainImages.data());

    for (u32 i = 0; i < imageCount; i++)
    {
      ICE_ATTEMPT(CreateImageView(&context.swapchainImageViews[i],
                                  context.swapchainImages[i],
                                  context.swapchainFormat,
                                  VK_IMAGE_ASPECT_COLOR_BIT));
    }
  }

  return true;
}

b8 Ice::RendererVulkan::CreateSyncObjects()
{
  VkSemaphoreCreateInfo semaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
  semaphoreInfo.flags = 0;

  VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
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

b8 Ice::RendererVulkan::CreateCommandBuffers()
{
  const u32 count = (u32)context.swapchainImages.size();
  context.commandBuffers.resize(count);

  VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = count;
  allocInfo.commandPool = context.graphicsCommandPool;

  IVK_ASSERT(vkAllocateCommandBuffers(context.device,
                                      &allocInfo,
                                      context.commandBuffers.data()),
             "Failed to allocate command buffers");

  return true;
}

b8 Ice::RendererVulkan::InitializeRenderComponent(Ice::RenderComponent* _component,
                                                  Ice::BufferSegment const _transformBuffer)
{
  ICE_ATTEMPT(CreateDescriptorSet(&context.objectDescriptorLayout,
                                  &_component->vulkan.descriptorSet));

  return UpdateShaderBindings(&_component->vulkan.descriptorSet, _transformBuffer);
}

b8 Ice::RendererVulkan::UpdateRenderComponent(Ice::RenderComponent* const _component,
                                              Ice::BufferSegment const _transformBufferSegment)
{
  return UpdateShaderBindings(&_component->vulkan.descriptorSet, _transformBufferSegment);
}

b8 Ice::RendererVulkan::UpdateCameraComponent(Ice::CameraComponent* const _camera,
                                              Ice::BufferSegment const _transformBufferSegment)
{
  Ice::BufferSegment segment;
  segment.buffer = &_camera->buffer;
  segment.elementSize = _camera->buffer.elementSize;
  segment.count = 1;
  segment.offset = 0;
  segment.startIndex = 0;

  return UpdateShaderBindings(&_camera->vulkan.descriptorSet, segment);
}

// TODO : Abstract descriptor bindings so they are easier to work with
b8 Ice::RendererVulkan::UpdateShaderBindings(VkDescriptorSet* const _set,
                                             Ice::BufferSegment const _transformBufferSegment)
{
  Ice::ShaderInputElement bufferInput;
  bufferInput.inputIndex = 0;
  bufferInput.type = Ice::Shader_Input_Buffer;
  bufferInput.bufferSegment = _transformBufferSegment;
  std::vector<Ice::ShaderInputElement> inputs = { bufferInput };
  UpdateDescriptorSet(_set, inputs);

  return true;
}

void Ice::RendererVulkan::DestroyRenderComponent(Ice::RenderComponent* _component)
{

}

b8 Ice::RendererVulkan::InitializeCamera(Ice::CameraComponent* _camera,
                                         Ice::BufferSegment _transformSegment,
                                         Ice::CameraSettings _settings)
{
  UpdateCameraProjection(_camera, _settings);

  ICE_ATTEMPT(CreateBufferMemory(&_camera->buffer,
                                 sizeof(Ice::CameraData),
                                 1,
                                 Ice::Buffer_Memory_Shader_Read | Ice::Buffer_Memory_Transfer_Dst));

  // Create descriptor set =====
  ICE_ATTEMPT(CreateDescriptorSet(&context.cameraDescriptorLayout, &_camera->vulkan.descriptorSet));

  Ice::BufferSegment segment;
  segment.buffer = &_camera->buffer;
  segment.elementSize = _camera->buffer.elementSize;
  segment.count = 1;
  segment.offset = 0;
  segment.startIndex = 0;

  return UpdateShaderBindings(&_camera->vulkan.descriptorSet, segment);
}

b8 Ice::RendererVulkan::UpdateCameraProjection(Ice::CameraComponent* _camera,
                                               Ice::CameraSettings _settings)
{
  // Calculate projection matrix =====
  glm::mat4 glmMatrix;
  if (_settings.isPerspective)
  {
    glmMatrix = glm::perspective(glm::radians(_settings.verticalFov),
                                 _settings.ratio,
                                 _settings.nearClip,
                                 _settings.farClip);
  }
  else
  {
    IceLogWarning("Orthographic camera funcionality not currently working");
    glmMatrix = glm::ortho(0.0f,
                           _settings.height * _settings.ratio,
                           0.0f,
                           _settings.height,
                           _settings.nearClip,
                           _settings.farClip);
  }
  glmMatrix[1][1] *= -1;

  _camera->projectionMatrix = Ice::mat4((f32*)glm::value_ptr(glmMatrix));

  _camera->settings = _settings;

  return true;
}
