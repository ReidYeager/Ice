
#include "defines.h"

#if defined(ICE_VULKAN)

#include <vulkan/vulkan.h>
#include <set>
#include <string>

#include "logger.h"
#include "renderer\renderer_backend.h"
#include "platform/platform.h"

RendererBackend::RendererBackend()
{
  CreateInstance();
  surface = Platform::CreateSurface(&instance);
  CreateDevice();
  CreateCommandPool();

  InitializeComponents();
}

RendererBackend::~RendererBackend()
{
  // Destroy command buffers
  // Destroy sync objects
  // Destroy descriptor pool
  DestroyComponents();

  vkDestroyCommandPool(vState.device, commandPool, nullptr);
  vkDestroyDevice(vState.device, nullptr);
  vkDestroySurfaceKHR(instance, surface, nullptr);
  vkDestroyInstance(instance, nullptr);
}

void RendererBackend::InitializeComponents()
{
  CreateComponents();

  //CreateDescriptorPool();
  //CreateSyncObjects();
  //CreateCommandBuffers();
}

void RendererBackend::CreateComponents()
{
  CreateSwapchain();
  //CreateRenderpass();
  //CreateDepthImage();
  //CreateFramebuffers();
}

void RendererBackend::DestroyComponents()
{
  // Destroy framebuffers
  // Destroy depth image
  // Destroy renderpass

  // Destroy swapchain
  for (const auto& view : swapchainImageViews)
  {
    vkDestroyImageView(vState.device, view, nullptr);
  }
  vkDestroySwapchainKHR(vState.device, swapchain, nullptr); // Implicitly destroys swapchainImages
}

void RendererBackend::RecreateComponents()
{
  // Wait for all frames to complete

  DestroyComponents();
  CreateComponents();
}

i8 RendererBackend::CreateInstance()
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

  if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
  {
    // ERROR
    return -1;
  }

  return 0;
}

i8 RendererBackend::CreateDevice()
{
  // Choose a physical device
  u32 queueIndices[3];
  u32 queueCount = 3;
  VkPhysicalDevice chosenPhysicalDevice = VK_NULL_HANDLE;
  ChoosePhysicalDevice(chosenPhysicalDevice, queueIndices[0], queueIndices[1], queueIndices[2]);

  IcePrint("Graphics : %u\nPresent : %u\nTransfer : %u\n",
         queueIndices[0], queueIndices[1], queueIndices[2]);

  // Fill physical device details
  vState.gpu.device = chosenPhysicalDevice;
  vState.graphicsIdx = queueIndices[0];
  vState.presentIdx  = queueIndices[1];
  vState.transferIdx = queueIndices[2];

  IcePhysicalDeviceInformation& dInfo = vState.gpu;
  VkPhysicalDevice& pDevice = vState.gpu.device;

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

  if (vkCreateDevice(pDevice, &createInfo, nullptr, &vState.device) != VK_SUCCESS)
  {
    IcePrint("Failed to create vkDevice\n");
    return -1;
  }

  vkGetDeviceQueue(vState.device, vState.graphicsIdx, 0, &vState.graphicsQueue);
  vkGetDeviceQueue(vState.device, vState.presentIdx , 0, &vState.presentQueue );
  vkGetDeviceQueue(vState.device, vState.transferIdx, 0, &vState.transferQueue);

  return 0;
}

i8 RendererBackend::ChoosePhysicalDevice(VkPhysicalDevice& _selectedDevice, u32& _graphicsIndex,
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
        return 0;
      }
    }
  }

  if (bestFit.device != VK_NULL_HANDLE)
  {
    _selectedDevice = bestFit.device;
    _graphicsIndex = bestFit.graphicsIndex;
    _presentIndex = bestFit.presentIndex;
    _transferIndex = bestFit.transferIndex;
    return 0;
  }

  // Error : Failed to find a suitable GPU
  return -1;
}

i8 RendererBackend::CreateCommandPool()
{
  VkCommandPoolCreateInfo createInfo {};
  createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  createInfo.queueFamilyIndex = vState.graphicsIdx;
  createInfo.flags = 0;
  if (vkCreateCommandPool(vState.device, &createInfo, nullptr, &commandPool) != VK_SUCCESS)
  {
    IcePrint("Failed to create command pool for queue family %u", vState.graphicsIdx);
    return -1;
  }

  return 0;
}

i8 RendererBackend::CreateSwapchain()
{
  // Find the best format
  VkSurfaceFormatKHR formatInfo = vState.gpu.surfaceFormats[0];
  for (const auto& sFormat : vState.gpu.surfaceFormats)
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
  for (const auto& pMode : vState.gpu.presentModes)
  {
    if (pMode == VK_PRESENT_MODE_MAILBOX_KHR)
    {
      presentMode = pMode;
      break;
    }
  }

  // Get the device's extent
  VkSurfaceCapabilitiesKHR& capabilities = vState.gpu.surfaceCapabilities;
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
  if (vState.graphicsIdx != vState.presentIdx)
  {
    u32 sharedIndices[] = {vState.graphicsIdx, vState.presentIdx};
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = sharedIndices;
  }
  else
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  if (vkCreateSwapchainKHR(vState.device, &createInfo, nullptr, &swapchain) != VK_SUCCESS)
  {
    IcePrint("Failed to create swapchain");
    return -1;
  }

  swapchainFormat = formatInfo.format;
  vState.renderExtent = extent;

  vkGetSwapchainImagesKHR(vState.device, swapchain, &imageCount, nullptr);
  swapchainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(vState.device, swapchain, &imageCount, swapchainImages.data());

  swapchainImageViews.resize(imageCount);
  for (u32 i = 0; i < imageCount; i++)
  {
    swapchainImageViews[i] =
        CreateImageView(swapchainFormat, VK_IMAGE_ASPECT_COLOR_BIT, swapchainImages[i]);

    if (swapchainImageViews[i] == VK_NULL_HANDLE)
    {
      return -1;
    }
  }

  return 0;
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
  if (vkCreateImageView(vState.device, &createInfo, nullptr, &createdView) != VK_SUCCESS)
  {
    IcePrint("Failed to create image view");
    return VK_NULL_HANDLE;
  }

  return createdView;
}

#endif // ICE_VULKAN
