
#include "defines.h"

#include "rendering/vulkan/vk_renderer.h"
#include "platform/platform.h"
#include "libraries/imgui/imgui.h"
#include "libraries/imgui/imgui_impl_vulkan.h"
#include "libraries/imgui/imgui_impl_win32.h"

VkDescriptorPool imguiPool;
ImGui_ImplVulkanH_Window imguiWindow;

b8 IvkRenderer::InitImgui()
{
  // Initialize IMGUI =====

  //1: create descriptor pool for IMGUI
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

  IVK_ASSERT(vkCreateDescriptorPool(context.device, &pool_info, nullptr, &imguiPool),
             "Failed to create descriptor pool for ImGui");

  // 2: initialize imgui library

  //this initializes the core structures of imgui
  ImGui::CreateContext();

  ImGui_ImplWin32_Init((void*)(rePlatform.GetVendorInfo()->hwnd));

  //this initializes imgui for Vulkan
  ImGui_ImplVulkan_InitInfo initInfo{};
  initInfo.Instance = context.instance;
  initInfo.PhysicalDevice = context.gpu.device;
  initInfo.Device = context.device;
  initInfo.Queue = context.graphicsQueue;
  initInfo.DescriptorPool = imguiPool;
  initInfo.MinImageCount = context.swapchainImages.size();
  initInfo.ImageCount = context.swapchainImages.size();
  initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  initInfo.Allocator = context.alloc;

  ImGui_ImplVulkan_Init(&initInfo, context.forwardRenderpass);

  ImVec2 windowSize;
  windowSize.x = context.swapchainExtent.width;
  windowSize.y = context.swapchainExtent.height;

  //execute a gpu command to upload imgui font textures
  VkCommandBuffer cmd = BeginSingleTimeCommand(context.graphicsCommandPool);
  ImGui_ImplVulkan_CreateFontsTexture(cmd);
  EndSingleTimeCommand(cmd, context.graphicsCommandPool, context.graphicsQueue);

  //clear font textures from cpu data
  ImGui_ImplVulkan_DestroyFontUploadObjects();

  return true;
}

void IvkRenderer::ShutdownImgui()
{
  vkDestroyDescriptorPool(context.device, imguiPool, nullptr);
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplWin32_Shutdown();
}

void IvkRenderer::RenderImgui(VkCommandBuffer& _cmdBuffer)
{
  ImGui::Render();
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), _cmdBuffer);
}
