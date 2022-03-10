
#include "defines.h"

#include "rendering/vulkan/vk_renderer.h"
#include "platform/platform.h"
#include "math/vector.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#include <vector>

#ifdef ICE_PLATFORM_WINDOWS

void IvkRenderer::GetRequiredPlatformExtensions(std::vector<const char*>& _extensions)
{
  _extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
}

b8 IvkRenderer::CreateSurface(zIceWindow* _window)
{
  //rePlatformVendorData const* vendorData = rePlatform.GetVendorInfo();
  zIceWindowVendorData const vendorData = _window->GetData();

  VkWin32SurfaceCreateInfoKHR createInfo { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
  createInfo.hinstance = vendorData.hinstance;
  createInfo.hwnd = vendorData.hwnd;
  createInfo.flags = 0;

  IVK_ASSERT(vkCreateWin32SurfaceKHR(context.instance,
                                     &createInfo,
                                     context.alloc,
                                     &context.surface),
             "Failed to create a Win32 surface");

  return context.surface != VK_NULL_HANDLE;
}

vec2U IvkRenderer::GetPlatformWindowExtents()
{
  return rePlatform.GetWindowInfo()->extents;
}

// End ICE_PLATFORM_WINDOWS
#else

void IvkRenderer::GetRequiredPlatformExtensions(std::vector<const char*>& _extensions) {}
b8 IvkRenderer::CreateSurface() { return false; }

#endif