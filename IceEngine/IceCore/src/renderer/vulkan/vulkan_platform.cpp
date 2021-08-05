
#include "asserts.h"
#include "service_hub.h"

#include "renderer/vulkan/vulkan_backend.h"
#include "platform/platform.h"

#include <vulkan/vulkan.h>

#include <vector>

#ifdef ICE_PLATFORM_WINDOWS
#include <vulkan/vulkan_win32.h>

void VulkanBackend::GetRequiredPlatformExtensions(std::vector<const char*>& _extensions)
{
  _extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
}

void VulkanBackend::CreateSurface()
{
  IcePlatform* platform = ServiceHub::GetPlatform();

  VkWin32SurfaceCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  createInfo.hinstance = platform->state.localState.hinstance;
  createInfo.hwnd = platform->state.localState.hwnd;
  createInfo.flags = 0;

  IVK_ASSERT(vkCreateWin32SurfaceKHR(
                 rContext->instance, &createInfo, rContext->allocator, &rContext->surface),
             "Failed to create win32 Vulkan surface");
}

VkExtent2D VulkanBackend::GetWindowExtent()
{
  VkSurfaceCapabilitiesKHR& capabilities = rContext->gpu.surfaceCapabilities;

  RECT rect;
  VkExtent2D extent;

  if (!GetWindowRect(ServiceHub::GetPlatform()->state.localState.hwnd, &rect))
  {
    LogError("Failed to get window rect");
    return rContext->renderExtent;
  }
  else
  {
    extent.width = rect.right - rect.left;
    extent.height = rect.bottom - rect.top;
  }

  if (capabilities.currentExtent.width != UINT32_MAX)
  {
    return capabilities.currentExtent;
  }
  else
  {
    IcePlatform* platform = ServiceHub::GetPlatform();
    RECT rect;
    ICE_ASSERT(GetWindowRect(platform->state.localState.hwnd, &rect));
    extent.width = rect.right - rect.left;
    extent.height = rect.bottom - rect.top;

    // TODO : Replace with a clamp function
    if (extent.width < capabilities.minImageExtent.width)
      extent.width = capabilities.minImageExtent.width;
    else if (extent.width > capabilities.maxImageExtent.width)
      extent.width = capabilities.maxImageExtent.width;

    if (extent.height < capabilities.minImageExtent.height)
      extent.height = capabilities.minImageExtent.height;
    else if (extent.height > capabilities.maxImageExtent.height)
      extent.height = capabilities.maxImageExtent.height;
  }

  LogInfo("Renderer : Window at (%u, %u)", extent.width, extent.height);
  return extent;
}

#else

void VulkanBackend::GetRequiredPlatformExtensions(std::vector<const char*>& _extensions)
{

}

void VulkanBackend::CreateSurface()
{
  LogFatal("This platform is unsupported");
}

VkExtent2D VulkanBackend::GetWindowExtent()
{
  VkExtent2D extent;
  return extent;
}

#endif // ICE_PLATFORM_WINDOWS



