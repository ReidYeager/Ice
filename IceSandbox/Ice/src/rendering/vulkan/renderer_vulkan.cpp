
#include "defines.h"
#include "asserts.h"
#include "logger.h"

#include "rendering/vulkan/renderer_vulkan.h"

#include "core/input.h"
#include "core/camera.h"
#include "platform/platform.h"
#include "platform/file_system.h"
#include "rendering/mesh.h"

#include <vector>

mesh_t mesh;

b8 IvkRenderer::Initialize()
{
  IceLogDebug("===== Vulkan Renderer Init =====");

  // API initialization =====

  ICE_ATTEMPT(CreateInstance());
  ICE_ATTEMPT(CreateSurface());
  ICE_ATTEMPT(ChoosePhysicalDevice());
  ICE_ATTEMPT(CreateLogicalDevice());

  // Rendering components =====

  ICE_ATTEMPT(CreateDescriptorPool());
  ICE_ATTEMPT(CreateCommandPool());
  ICE_ATTEMPT(CreateCommandPool(true));

  ICE_ATTEMPT(CreateSwapchain());
  ICE_ATTEMPT(CreateDepthImage());
  ICE_ATTEMPT(CreateRenderpass());
  ICE_ATTEMPT(CreateFrameBuffers());

  ICE_ATTEMPT(CreateSyncObjects());
  ICE_ATTEMPT(CreateCommandBuffers());

  IceLogDebug("===== Vulkan Renderer Init Complete =====");

  // TMP =====
  CreateMesh(&mesh, "Sphere.obj");

  return true;
}

b8 IvkRenderer::Shutdown()
{
  vkDeviceWaitIdle(context.device);

  // TMP =====
  DestroyBuffer(&viewProjBuffer, true);
  DestroyBuffer(&mesh.vertBuffer, true);
  DestroyBuffer(&mesh.indexBuffer, true);

  // Material =====
  for (const auto& mat : materials)
  {
    vkDestroyShaderModule(context.device, mat.vertexModule.module, context.alloc);
    vkDestroyShaderModule(context.device, mat.fragmentModule.module, context.alloc);

    vkDestroyPipeline(context.device, mat.pipeline, context.alloc);
    vkDestroyPipelineLayout(context.device, mat.pipelineLayout, context.alloc);
    vkDestroyDescriptorSetLayout(context.device, mat.descriptorSetLayout, context.alloc);
  }


  // Rendering components =====
  vkFreeCommandBuffers(context.device,
                       context.graphicsCommandPool,
                       context.commandsBuffers.size(),
                       context.commandsBuffers.data());

  for (u32 i = 0; i < RE_MAX_FLIGHT_IMAGE_COUNT; i++)
  {
    vkDestroyFence(context.device, context.flightSlotAvailableFences[i], context.alloc);
    vkDestroySemaphore(context.device, context.renderCompleteSemaphores[i], context.alloc);
    vkDestroySemaphore(context.device, context.imageAvailableSemaphores[i], context.alloc);
  }

  for (const auto& f : context.frameBuffers)
  {
    vkDestroyFramebuffer(context.device, f, context.alloc);
  }

  vkDestroyRenderPass(context.device, context.renderpass, context.alloc);

  vkDestroyImage(context.device, context.depthImage.image, context.alloc);
  vkDestroyImageView(context.device, context.depthImage.view, context.alloc);
  vkFreeMemory(context.device, context.depthImage.memory, context.alloc);

  for (const auto& v : context.swapchainImageViews)
  {
    vkDestroyImageView(context.device, v, context.alloc);
  }

  vkDestroySwapchainKHR(context.device, context.swapchain, context.alloc);

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
  static u32 flightSlotIndex = 0;
  static u32 swapchainImageIndex = 0;

  // Wait for oldest in-flight slot to return (FIFO) =====
  IVK_ASSERT(vkWaitForFences(context.device,
                             1,
                             &context.flightSlotAvailableFences[flightSlotIndex],
                             VK_TRUE,
                             3000000000), // 3 second timeout
             "Flight slot wait fence failed");

  IVK_ASSERT(vkAcquireNextImageKHR(context.device,
                                   context.swapchain,
                                   UINT64_MAX,
                                   context.imageAvailableSemaphores[flightSlotIndex],
                                   VK_NULL_HANDLE,
                                   &swapchainImageIndex),
             "Failed to acquire the next swapchain image");

  // TMP camera =====
  FillBuffer(&viewProjBuffer, (void*)&_camera->viewProjectionMatrix, 64);

  // Submit a command buffer =====
  RecordCommandBuffer(swapchainImageIndex);

  VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

  VkSubmitInfo submitInfo { VK_STRUCTURE_TYPE_SUBMIT_INFO };
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &context.imageAvailableSemaphores[flightSlotIndex];
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &context.commandsBuffers[swapchainImageIndex];
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

  IVK_ASSERT(vkQueuePresentKHR(context.presentQueue, &presentInfo),
             "Failed to present the swapchain");

  flightSlotIndex = (flightSlotIndex + 1) % RE_MAX_FLIGHT_IMAGE_COUNT;

  return true;
}

b8 IvkRenderer::CreateInstance()
{
  // Retrieve and validate extensions and layers to enable
  std::vector<const char*> extensions;
  GetPlatformExtensions(extensions);
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
  reIvkGpu& gpu = context.gpu;

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

  IceLogInfo("Queue Family Indicies -- Graphics : %u -- Presentation : %u -- Transfer : %u",
              queueIndices[0], queueIndices[1], queueIndices[2]);

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

b8 IvkRenderer::CreateDescriptorPool()
{
  // Size definitions =====
  const u32 poolSizeCount = 2;
  VkDescriptorPoolSize sizes[poolSizeCount];

  sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  sizes[0].descriptorCount = 100;

  sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  sizes[1].descriptorCount = 100;

  // Creation =====
  VkDescriptorPoolCreateInfo createInfo { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
  createInfo.flags = 0;
  createInfo.maxSets = 4; // (max possible) 0: Global, 1: per-shader, 2: per-material, 3: per-object
  createInfo.poolSizeCount = poolSizeCount;
  createInfo.pPoolSizes = sizes;

  IVK_ASSERT(vkCreateDescriptorPool(context.device,
                                    &createInfo,
                                    context.alloc,
                                    &context.descriptorPool),
             "Failed to create descriptor pool");

  return true;
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

    // TODO : Remove this and manually hand-off ownership of the images
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

    // Swapchain context =====
    context.swapchainFormat = format.format;
    context.swapchainExtent = extent;
  }

  // Retrieve images and create views =====
  {
    // Get the number of images actually created
    vkGetSwapchainImagesKHR(context.device, context.swapchain, &imageCount, nullptr);

    IceLogInfo("Created %u images for the swapchain", imageCount);

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

b8 IvkRenderer::CreateRenderpass()
{
  // Color =====
  VkAttachmentDescription colorDescription {};
  colorDescription.format = context.swapchainFormat;
  colorDescription.samples = VK_SAMPLE_COUNT_1_BIT;
  colorDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorDescription.finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  colorDescription.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  VkAttachmentReference colorReference {};
  colorReference.attachment = 0;
  colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // Depth =====
  VkAttachmentDescription depthDescription {};
  depthDescription.format = context.depthImage.format;
  depthDescription.samples = VK_SAMPLE_COUNT_1_BIT;
  depthDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthDescription.finalLayout   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  depthDescription.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  VkAttachmentReference depthReference {};
  depthReference.attachment = 1;
  depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  // Subpass =====
  VkSubpassDescription subpassDescription {};
  subpassDescription.colorAttachmentCount = 1;
  subpassDescription.pColorAttachments = &colorReference;
  subpassDescription.pDepthStencilAttachment = &depthReference;
  subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

  VkSubpassDependency subpassDependency {};
  subpassDependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
  subpassDependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.srcAccessMask = 0;
  subpassDependency.dstSubpass    = 0;
  subpassDependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  // Creation =====
  const u32 attachemntCount = 2;
  VkAttachmentDescription attachments[attachemntCount] = { colorDescription, depthDescription };
  const u32 subpassCount = 1;
  VkSubpassDescription subpasses[subpassCount] = { subpassDescription };
  const u32 dependencyCount = 1;
  VkSubpassDependency dependencies[dependencyCount] = { subpassDependency };

  VkRenderPassCreateInfo createInfo { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
  createInfo.flags = 0;
  createInfo.attachmentCount = attachemntCount;
  createInfo.pAttachments    = attachments;
  createInfo.subpassCount = subpassCount;
  createInfo.pSubpasses   = subpasses;
  createInfo.dependencyCount = dependencyCount;
  createInfo.pDependencies   = dependencies;

  IVK_ASSERT(vkCreateRenderPass(context.device, &createInfo, context.alloc, &context.renderpass),
             "Failed to create renderpass");

  return true;
}

b8 IvkRenderer::CreateDepthImage()
{
  // Find depth format =====
  VkFormat format = VK_FORMAT_D32_SFLOAT;
  VkFormatProperties formatProperties;
  const u32 fCount = 3;
  VkFormat desiredFormats[fCount] = { VK_FORMAT_D32_SFLOAT,
                                      VK_FORMAT_D32_SFLOAT_S8_UINT,
                                      VK_FORMAT_D24_UNORM_S8_UINT };
  for (u32 i = 0; i < fCount; i++)
  {
    vkGetPhysicalDeviceFormatProperties(context.gpu.device, desiredFormats[i], &formatProperties);

    if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
      format = desiredFormats[i];
      break;
    }
  }

  context.depthImage.format = format;

  // Create image =====
  ICE_ATTEMPT(CreateImage(&context.depthImage,
                          context.swapchainExtent,
                          format,
                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT));

  // Create image view =====
  ICE_ATTEMPT(CreateImageView(&context.depthImage.view,
                              context.depthImage.image,
                              context.depthImage.format,
                              VK_IMAGE_ASPECT_DEPTH_BIT));

  return true;
}

b8 IvkRenderer::CreateFrameBuffers()
{
  VkFramebufferCreateInfo createInfo { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
  createInfo.flags = 0;
  createInfo.layers = 1;
  createInfo.renderPass = context.renderpass;
  createInfo.width  = context.swapchainExtent.width;
  createInfo.height = context.swapchainExtent.height;

  // One framebuffer per frame
  const u32 imageCount = context.swapchainImages.size();
  context.frameBuffers.resize(imageCount);

  for (u32 i = 0; i < imageCount; i++)
  {
    VkImageView attachments[] = { context.swapchainImageViews[i], context.depthImage.view };
    createInfo.attachmentCount = 2;
    createInfo.pAttachments = attachments;

    IVK_ASSERT(vkCreateFramebuffer(context.device,
                                   &createInfo,
                                   context.alloc,
                                   &context.frameBuffers[i]),
               "Failed to create frame buffer %u", i);
  }

  return true;
}

b8 IvkRenderer::CreateSyncObjects()
{
  context.imageAvailableSemaphores.resize(RE_MAX_FLIGHT_IMAGE_COUNT);
  context.renderCompleteSemaphores.resize(RE_MAX_FLIGHT_IMAGE_COUNT);
  context.flightSlotAvailableFences.resize(RE_MAX_FLIGHT_IMAGE_COUNT);

  VkSemaphoreCreateInfo semaphoreInfo { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
  semaphoreInfo.flags = 0;

  VkFenceCreateInfo fenceInfo { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (u32 i = 0; i < RE_MAX_FLIGHT_IMAGE_COUNT; i++)
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

b8 IvkRenderer::CreateCommandBuffers()
{
  const u32 count = context.swapchainImages.size();
  context.commandsBuffers.resize(count);

  VkCommandBufferAllocateInfo allocInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = count;
  allocInfo.commandPool = context.graphicsCommandPool;

  IVK_ASSERT(vkAllocateCommandBuffers(context.device,
                                      &allocInfo,
                                      context.commandsBuffers.data()),
             "Failed to allocate command buffers");

  return true;
}

b8 IvkRenderer::RecordCommandBuffer(u32 _commandIndex)
{
  VkCommandBufferBeginInfo beginInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
  beginInfo.flags = 0;

  VkClearValue clearValues[2] = {};
  clearValues[0].color = { 0.3f, 0.3f, 0.3f };
  clearValues[1].depthStencil = { 1, 0 };

  VkRenderPassBeginInfo renderPassBeginInfo { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
  renderPassBeginInfo.clearValueCount = 2;
  renderPassBeginInfo.pClearValues = clearValues;
  renderPassBeginInfo.renderArea.extent = context.swapchainExtent;
  renderPassBeginInfo.renderArea.offset = { 0 , 0 };
  renderPassBeginInfo.renderPass = context.renderpass;

  VkCommandBuffer& cmdBuffer = context.commandsBuffers[_commandIndex];

  renderPassBeginInfo.framebuffer = context.frameBuffers[_commandIndex];

  IVK_ASSERT(vkBeginCommandBuffer(cmdBuffer, &beginInfo),
             "Failed to begin command buffer %u", _commandIndex);

  vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

  // -- For each view
  // Bind global descriptors (cam view, environmental info, lights, etc.) == set 0

  for (const auto& mat : materials)
  {
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mat.pipeline);
    vkCmdBindDescriptorSets(cmdBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            mat.pipelineLayout,
                            0,
                            1,
                            &mat.descriptorSet,
                            0,
                            nullptr);

    // -- For each material instance
    // Bind descriptor set == Set 2

    // -- For each object
    // bind push constants (if any)
    // bind vertices & indices
    // render
    VkDeviceSize zero = 0;
    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &mesh.vertBuffer.buffer, &zero);
    vkCmdBindIndexBuffer(cmdBuffer, mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(cmdBuffer, mesh.indices.size(), 1, 0, 0, 0);
  //vkCmdDraw(cmdBuffer, 3, 1, 0, 0);

  }

  vkCmdEndRenderPass(cmdBuffer);
  IVK_ASSERT(vkEndCommandBuffer(cmdBuffer),
             "Failed to record command buffer %u", _commandIndex);

  return true;
}

b8 IvkRenderer::CreateMesh(mesh_t* _mesh, const char* _directory)
{
  *_mesh = FileSystem::LoadMesh(_directory);

  CreateBuffer(&_mesh->vertBuffer,
               _mesh->vertices.size() * sizeof(iceVertex),
               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               (void*)_mesh->vertices.data());
  CreateBuffer(&_mesh->indexBuffer,
               _mesh->indices.size() * sizeof(u32),
               VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               (void*)_mesh->indices.data());

  return true;
}
