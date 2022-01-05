
#include "defines.h"
#include "asserts.h"
#include "logger.h"

#include "rendering/vulkan/vulkan_renderer.h"

#include "core/input.h"
#include "core/camera.h"
#include "platform/platform.h"
#include "platform/file_system.h"
#include "rendering/mesh.h"

#include "libraries/imgui/imgui.h"
#include "libraries/imgui/imgui_impl_vulkan.h"
#include "libraries/imgui/imgui_impl_win32.h"

#include <vector>

// TODO : Deferred rendering
//   https://docs.microsoft.com/en-us/windows/win32/fileio/obtaining-directory-change-notifications
// TODO : Integrate cameras & lights into this scene system

b8 IvkRenderer::Initialize()
{
  IceLogDebug("===== Vulkan Renderer Init =====");

  tmpLights.directional.color = { 1.0f, 1.0f, 1.0f };
  tmpLights.directional.direction = { -1.0f, -1.0f, -1.0f };

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
  ICE_ATTEMPT(CreateMainRenderPass());

  ICE_ATTEMPT(CreateMainFrameBuffers());

  ICE_ATTEMPT(CreateShadowImages());
  ICE_ATTEMPT(CreateShadowRenderpass());
  ICE_ATTEMPT(CreateShadowFrameBuffer());

  ICE_ATTEMPT(PrepareGlobalDescriptors());
  ICE_ATTEMPT(PrepareShadowDescriptors());

  ICE_ATTEMPT(CreateSyncObjects());
  ICE_ATTEMPT(CreateCommandBuffers());

  IceLogDebug("===== Vulkan Renderer Init Complete =====");

  // Initialize IMGUI =====
  {
    //1: create descriptor pool for IMGUI
  // the size of the pool is very oversize, but it's copied from imgui demo itself.
    VkDescriptorPoolSize pool_sizes[] =
    {
      { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
      { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
      { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
      { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
      { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
      { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
      { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
      { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
      { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
      { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
      { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    IVK_ASSERT(vkCreateDescriptorPool(context.device, &pool_info, nullptr, &context.imguiPool),
               "Failed to create descriptor pool for ImGui");

    // 2: initialize imgui library

    //this initializes the core structures of imgui
    ImGui::CreateContext();

    ImGui_ImplWin32_Init((void*)(rePlatform.GetVendorInfo()->hwnd));

    //this initializes imgui for Vulkan
    ImGui_ImplVulkan_InitInfo initInfo {};
    initInfo.Instance = context.instance;
    initInfo.PhysicalDevice = context.gpu.device;
    initInfo.Device = context.device;
    initInfo.Queue = context.graphicsQueue;
    initInfo.DescriptorPool = context.imguiPool;
    initInfo.MinImageCount = context.swapchainImages.size();
    initInfo.ImageCount = context.swapchainImages.size();
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    initInfo.Allocator = context.alloc;

    ImGui_ImplVulkan_Init(&initInfo, context.mainRenderpass);

    ImVec2 windowSize;
    windowSize.x = context.swapchainExtent.width;
    windowSize.y = context.swapchainExtent.height;

    //execute a gpu command to upload imgui font textures
    VkCommandBuffer cmd = BeginSingleTimeCommand(context.graphicsCommandPool);
    ImGui_ImplVulkan_CreateFontsTexture(cmd);
    EndSingleTimeCommand(cmd, context.graphicsCommandPool, context.graphicsQueue);

    //clear font textures from cpu data
    ImGui_ImplVulkan_DestroyFontUploadObjects();
  }

  return true;
}

b8 IvkRenderer::Shutdown()
{
  vkDeviceWaitIdle(context.device);

  // TMP =====
  DestroyBuffer(&globalDescriptorBuffer);

  // ImGui =====
  vkDestroyDescriptorPool(context.device, context.imguiPool, nullptr);
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplWin32_Shutdown();

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
  DestroyImage(&texture);

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
  vkDestroyPipelineLayout(context.device, context.globalPipelinelayout, context.alloc);
  vkDestroyDescriptorSetLayout(context.device, context.globalDescriptorSetLayout, context.alloc);

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

  vkDestroyRenderPass(context.device, context.mainRenderpass, context.alloc);

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
  {
    glm::mat4& proj = shadow.viewProjMatrix;
    proj = glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, -100.0f, 100.0f);
    proj[1][1] *= -1;
    proj = proj * glm::lookAt(glm::vec3(tmpLights.directional.direction.x,
                                        tmpLights.directional.direction.y,
                                        tmpLights.directional.direction.z) * -10.0f,
                              glm::vec3(0, 0, 0),
                              glm::vec3(0, 1, 0));

    FillBuffer(&globalDescriptorBuffer, (void*)&shadow.viewProjMatrix, 64, 64 + sizeof(IvkLights));
    FillBuffer(&shadow.lightMatrixBuffer, (void*)&shadow.viewProjMatrix, 64);
  }

  {
    ImGui::Begin("Directional light");
    ImGui::InputFloat3("Light color", &tmpLights.directional.color.x);
    ImGui::SliderFloat("direction x", &tmpLights.directional.direction.x, -1.0f, 1.0f);
    ImGui::SliderFloat("direction y", &tmpLights.directional.direction.y, -1.0f, 1.0f);
    ImGui::SliderFloat("direction z", &tmpLights.directional.direction.z, -1.0f, 1.0f);

    // normalize direction
    f32 xsq = tmpLights.directional.direction.x * tmpLights.directional.direction.x;
    f32 ysq = tmpLights.directional.direction.y * tmpLights.directional.direction.y;
    f32 zsq = tmpLights.directional.direction.z * tmpLights.directional.direction.z;
    f32 sq = xsq + ysq + zsq;
    sq = sqrt(sq);
    tmpLights.directional.direction.x /= sq;
    tmpLights.directional.direction.y /= sq;
    tmpLights.directional.direction.z /= sq;

    ImGui::End();
  }

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
  FillBuffer(&globalDescriptorBuffer, (void*)&_camera->viewProjectionMatrix, 64);
  FillBuffer(&globalDescriptorBuffer, (void*)&tmpLights, sizeof(IvkLights), 64);

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

b8 IvkRenderer::CreateSyncObjects()
{
  VkSemaphoreCreateInfo semaphoreInfo { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
  semaphoreInfo.flags = 0;

  VkFenceCreateInfo fenceInfo { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  // Create each frame's sync objects =====
  context.imageAvailableSemaphores.resize(RE_MAX_FLIGHT_IMAGE_COUNT);
  context.renderCompleteSemaphores.resize(RE_MAX_FLIGHT_IMAGE_COUNT);
  context.flightSlotAvailableFences.resize(RE_MAX_FLIGHT_IMAGE_COUNT);

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

b8 IvkRenderer::CreateMainRenderPass()
{
  // Attachments =====
  //std::vector<IvkAttachmentSettings> attachSettings(2);
  // Color
  //attachSettings[0].imageFormat = context.swapchainFormat;
  //attachSettings[0].finalLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  //attachSettings[0].referenceLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  //// Depth
  //attachSettings[1].imageFormat = context.depthImage.format;
  //attachSettings[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  //attachSettings[1].referenceLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription attachments[2];
  attachments[0].flags = 0;
  attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[0].format = context.swapchainFormat;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[0].finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  attachments[0].loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  attachments[1].flags = 0;
  attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[1].format = context.depthImage.format;
  attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[1].finalLayout   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  attachments[1].loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  VkAttachmentReference references[2];
  references[0].attachment = 0;
  references[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  references[1].attachment = 1;
  references[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  // Subpass =====
  //IvkSubpassSettings subpasses;
  //subpasses.colorIndices.push_back(0);
  //subpasses.depthIndex = 1;

  VkSubpassDescription subpass;
  subpass.flags = 0;
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &references[0];
  subpass.pDepthStencilAttachment = &references[1];
  subpass.inputAttachmentCount = 0;
  subpass.pInputAttachments = nullptr;
  subpass.preserveAttachmentCount = 0;
  subpass.pPreserveAttachments = nullptr;
  subpass.pResolveAttachments = nullptr;

  // Dependencies =====
  //IvkSubpassDependencies dependency {};
  //dependency.srcIndex = VK_SUBPASS_EXTERNAL;
  //dependency.dstIndex = 0;

  VkSubpassDependency dependencies[2];

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    // Creation =====
  VkRenderPassCreateInfo createInfo { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
  createInfo.flags = 0;
  createInfo.attachmentCount = 2;
  createInfo.pAttachments    = attachments;
  createInfo.subpassCount = 1;
  createInfo.pSubpasses   = &subpass;
  createInfo.dependencyCount = 2;
  createInfo.pDependencies   = dependencies;

  IVK_ASSERT(vkCreateRenderPass(context.device, &createInfo, context.alloc, &context.mainRenderpass),
             "Failed to create renderpass");

  return true;
}

b8 IvkRenderer::CreateShadowImages()
{
  ICE_ATTEMPT(CreateImage(&shadow.image,
                          { shadowResolution, shadowResolution },
                          VK_FORMAT_D16_UNORM,
                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT|VK_IMAGE_USAGE_SAMPLED_BIT));
  shadow.image.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

  ICE_ATTEMPT(CreateImageView(&shadow.image.view,
                              shadow.image.image,
                              shadow.image.format,
                              VK_IMAGE_ASPECT_DEPTH_BIT));

  ICE_ATTEMPT(CreateImageSampler(&shadow.image));

  return true;
}

b8 IvkRenderer::CreateShadowRenderpass()
{
  // Attachments =====
  VkAttachmentDescription attachSettings {};
  attachSettings.format = shadow.image.format;
  attachSettings.flags = 0;
  attachSettings.samples = VK_SAMPLE_COUNT_1_BIT;
  attachSettings.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachSettings.finalLayout   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
  attachSettings.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachSettings.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachSettings.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachSettings.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  VkAttachmentReference reference {};
  reference.attachment = 0;
  reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  // Subpass =====
  VkSubpassDescription subpass {};
  subpass.flags = 0;
  subpass.colorAttachmentCount = 0;
  subpass.pDepthStencilAttachment = &reference;
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

  // Dependencies =====
  VkSubpassDependency dependencies[2];
  dependencies[0].srcSubpass  = VK_SUBPASS_EXTERNAL;
  dependencies[0].dstSubpass = 0;
  dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
  dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  dependencies[1].srcSubpass = 0;
  dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  // Creation =====
  VkRenderPassCreateInfo createInfo { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
  createInfo.flags = 0;
  createInfo.attachmentCount = 1;
  createInfo.pAttachments    = &attachSettings;
  createInfo.subpassCount = 1;
  createInfo.pSubpasses   = &subpass;
  createInfo.dependencyCount = 2;
  createInfo.pDependencies   = dependencies;

  IVK_ASSERT(vkCreateRenderPass(context.device, &createInfo, context.alloc, &shadow.renderpass),
             "Failed to create shadow renderpass");

  return true;
}

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
