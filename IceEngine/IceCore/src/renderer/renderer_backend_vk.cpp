
#include "renderer\renderer_backend.h"

#if defined(ICE_VULKAN)

#include "vulkan/vulkan.h"

RendererBackend::RendererBackend()
{
  CreateInstance();
  // Create vkSurface
  // Create device
  // Create command pool
}

RendererBackend::~RendererBackend()
{
  vkDestroyInstance(instance, nullptr);
}

i8 RendererBackend::CreateComponents()
{
  return 0;
}

i8 RendererBackend::DestroyComponents()
{
  return 0;
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
  createInfo.enabledExtensionCount = 0;
  createInfo.ppEnabledExtensionNames = nullptr;
  createInfo.enabledLayerCount = 0;
  createInfo.ppEnabledLayerNames = nullptr;

  if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
  {
    // ERROR
    return -1;
  }

  return 0;
}

#endif // ICE_VULKAN
