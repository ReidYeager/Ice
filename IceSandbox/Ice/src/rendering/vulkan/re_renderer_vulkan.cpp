
#include "defines.h"
#include "asserts.h"
#include "logger.h"

#include "rendering/vulkan/re_renderer_vulkan.h"
#include "platform/re_platform.h"

#include <vector>

b8 reIvkRenderer::Initialize()
{
  IceLogDebug("===== Vulkan Renderer Init =====");

  if (!CreateInstance())
    return false;
  if (!CreateSurface())
    return false;
  if (!ChoosePhysicalDevice())
    return false;
  if (!CreateLogicalDevice())
    return false;

  IceLogDebug("===== Vulkan Renderer Init Complete =====")

  return true;
}

b8 reIvkRenderer::Shutdown()
{
  vkDestroyDevice(context.device, context.alloc);
  vkDestroySurfaceKHR(context.instance, context.surface, context.alloc);
  vkDestroyInstance(context.instance, context.alloc);
  return true;
}

b8 reIvkRenderer::Render()
{
  return true;
}

b8 reIvkRenderer::CreateInstance()
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

  // TODO : Define this information elsewhere
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

b8 reIvkRenderer::ChoosePhysicalDevice()
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
  gpu.surfaceFormats.resize(count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.device,
                                            context.surface,
                                            &count,
                                            gpu.presentModes.data());

  // Select queue families
  gpu.graphicsQueueIndex = GetIndexForQueue(VK_QUEUE_GRAPHICS_BIT);
  gpu.transferQueueIndex = GetIndexForQueue(VK_QUEUE_TRANSFER_BIT);
  gpu.presentQueueIndex = GetPresentQueue();

  if (gpu.graphicsQueueIndex == -1u ||
      gpu.presentQueueIndex == -1u  ||
      gpu.transferQueueIndex == -1u)
  {
    IceLogError("Some of the queue indices are invalid (G : %u, P : %u, T : %u)",
                gpu.graphicsQueueIndex,
                gpu.presentQueueIndex,
                gpu.transferQueueIndex);
    return false;
  }

  return true;
}

u32 reIvkRenderer::GetIndexForQueue(VkQueueFlags _flags)
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

u32 reIvkRenderer::GetPresentQueue()
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

b8 reIvkRenderer::IsDeviceSuitable(const VkPhysicalDevice& _device)
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

b8 reIvkRenderer::CreateLogicalDevice()
{
  // Use to enable features for use in rendering
  VkPhysicalDeviceFeatures enabledFeatures{};

  // Queues =====
  // Graphics, Presentation, Transfer
  const u32 queueCount = 3;
  const float queuePriority = 1.0f;

  u32 queueIndices[queueCount] = { context.gpu.graphicsQueueIndex,
                                   context.gpu.presentQueueIndex,
                                   context.gpu.transferQueueIndex };

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

  VkDeviceCreateInfo createInfo { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
  createInfo.flags = 0;
  createInfo.pEnabledFeatures = &enabledFeatures;
  createInfo.queueCreateInfoCount = queueCreateInfos.size(); // Doesn't work if it isn't a vector??
  createInfo.pQueueCreateInfos    = queueCreateInfos.data(); // Doesn't work if it isn't a vector??
  createInfo.enabledExtensionCount   = extensions.size();
  createInfo.ppEnabledExtensionNames = extensions.data();
  createInfo.enabledLayerCount   = layers.size();
  createInfo.ppEnabledLayerNames = layers.data();

  IVK_ASSERT(vkCreateDevice(context.gpu.device, &createInfo, nullptr, &context.device),
             "Failed to create Vulkan logical device");

  vkGetDeviceQueue(context.device, context.gpu.graphicsQueueIndex, 0, &context.graphicsQueue);
  vkGetDeviceQueue(context.device, context.gpu.presentQueueIndex , 0, &context.presentQueue );
  vkGetDeviceQueue(context.device, context.gpu.transferQueueIndex, 0, &context.transferQueue);

  return context.device != VK_NULL_HANDLE;
}
