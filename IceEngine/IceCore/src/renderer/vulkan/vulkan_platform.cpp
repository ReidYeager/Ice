
#include "asserts.h"
#include "service_hub.h"

#include "renderer/vulkan/vulkan_backend.h"
#include "platform/platform.h"

#include <vector>
#include <algorithm>

#include <vulkan/vulkan.h>
#ifdef ICE_PLATFORM_WINDOWS
#include <vulkan/vulkan_win32.h>

void VulkanBackend::GetRequiredPlatformExtensions(std::vector<const char*>& _extensions)
{
  // Required to create a surface on WINAPI
  _extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
}

void VulkanBackend::CreateSurface()
{
  // This feels bad and I don't like it.
  // Required to access windows' window information
  IcePlatform* platform = ServiceHub::GetPlatform();

  VkWin32SurfaceCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  createInfo.hinstance = platform->state.localState.hinstance;
  createInfo.hwnd = platform->state.localState.hwnd;
  createInfo.flags = 0;

  IVK_ASSERT(vkCreateWin32SurfaceKHR(rContext->instance,
                                     &createInfo,
                                     rContext->allocator,
                                     &rContext->surface),
             "Failed to create win32 Vulkan surface");
}

VkExtent2D VulkanBackend::GetWindowExtent()
{
  VkSurfaceCapabilitiesKHR& capabilities = rContext->gpu.surfaceCapabilities;

  RECT rect;
  VkExtent2D extent;

  if (!GetWindowRect(ServiceHub::GetPlatform()->state.localState.hwnd, &rect))
  {
    IceLogError("Failed to get window rect");
    return rContext->renderExtent;
  }
  else
  {
    extent.width = rect.right - rect.left;
    extent.height = rect.bottom - rect.top;
  }

  // Make sure the retrieved extent is valid
  if (capabilities.currentExtent.width != UINT32_MAX)
  {
    return capabilities.currentExtent;
  }
  else
  {
    // Get the current window size;
    IcePlatform* platform = ServiceHub::GetPlatform();
    RECT rect;
    ICE_ASSERT(GetWindowRect(platform->state.localState.hwnd, &rect));
    extent.width = rect.right - rect.left;
    extent.height = rect.bottom - rect.top;

    extent.width = std::clamp(extent.width,
                              capabilities.minImageExtent.width,
                              capabilities.maxImageExtent.width);

    extent.height = std::clamp(extent.height,
                               capabilities.minImageExtent.height,
                               capabilities.maxImageExtent.height);
  }

  IceLogInfo("Renderer : Window at (%u, %u)", extent.width, extent.height);
  return extent;
}

#else

void VulkanBackend::GetRequiredPlatformExtensions(std::vector<const char*>& _extensions)
{

}

void VulkanBackend::CreateSurface()
{
  IceLogFatal("This platform is unsupported");
}

VkExtent2D VulkanBackend::GetWindowExtent()
{
  VkExtent2D extent;
  return extent;
}

#endif // ICE_PLATFORM_WINDOWS



