
#include "defines.h"

#include "rendering/vulkan/vulkan.h"

#include "core/platform/platform.h"
#include "math/vector.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#ifdef ICE_PLATFORM_WINDOWS

void Ice::RendererVulkan::GetRequiredPlatformExtensions(std::vector<const char*>& _extensions)
{
  _extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
}

b8 Ice::RendererVulkan::CreateSurface()
{
  Ice::Window const* window = Ice::platform.GetWindow();

  VkWin32SurfaceCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
  createInfo.hinstance = window->platformData.hinstance;
  createInfo.hwnd = window->platformData.hwnd;
  createInfo.flags = 0;

  IVK_ASSERT(vkCreateWin32SurfaceKHR(context.instance,
                                     &createInfo,
                                     context.alloc,
                                     &context.surface),
             "Failed to create a Win32 surface");

  return context.surface != VK_NULL_HANDLE;
}

Ice::vec2U Ice::RendererVulkan::GetWindowExtents()
{
  return Ice::platform.GetWindow()->settings.extents;
}

#else
void Ice::RendererVulkan::GetRequiredPlatformExtensions(std::vector<const char*>& _extensions)
{}

b8 Ice::RendererVulkan::CreateSurface()
{
  return false;
}

Ice::vec2U Ice::RendererVulkan::GetWindowExtents()
{
  return { 0, 0 };
}
#endif

