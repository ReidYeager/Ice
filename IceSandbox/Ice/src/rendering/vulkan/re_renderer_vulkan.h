
#ifndef ICE_RENDERING_VULKAN_RE_RENDERER_VULKAN_H_
#define ICE_RENDERING_VULKAN_RE_RENDERER_VULKAN_H_

#include "defines.h"

#include "rendering/vulkan/re_renderer_vulkan_context.h"
#include "rendering/re_renderer_backend.h"

#include <vulkan/vulkan.h>

#include <vector>

class reIvkRenderer : public reIceRendererBackend
{
private:
  reIvkContext context;

public:
  b8 Initialize() override;
  b8 Shutdown() override;
  b8 Render() override;

private:
  // Ensures that all desired layer and extension functionality is present in the created instance
  b8 CreateInstance();
  // Retrieves all of the extensions the platform requires to render and present with Vulkan
  void GetPlatformExtensions(std::vector<const char*>& _extensions);
  // Creates a vendor-specific surface for display
  b8 CreateSurface();

  // Selects the first compatible GPU among those available in the system
  // Fills the selected GPU's information
  b8 ChoosePhysicalDevice();
  // Returns the first queue that satisfies the input flags
  u32 GetIndexForQueue(VkQueueFlags _flags);
  // Finds the first GPU queue that supports presentation
  u32 GetPresentQueue();
  // Tests the GPU for desired functionality
  b8 IsDeviceSuitable(const VkPhysicalDevice& _device);

  // ...
  b8 CreateLogicalDevice();

};

#endif // !define ICE_RENDERING_RE_RENDERER_VULKAN_H_
