
#ifndef RENDERER_BACKEND_CONTEXT_H
#define RENDERER_BACKEND_CONTEXT_H 1

#include "defines.h"

#ifdef ICE_VULKAN
#include "logger.h"
#include "renderer/shader_program.h"
#include <vulkan/vulkan.h>
#include <vector>
struct IcePhysicalDeviceInformation
{
  VkPhysicalDevice device;
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceMemoryProperties memProperties;
  VkPhysicalDeviceFeatures features;
  VkSurfaceCapabilitiesKHR surfaceCapabilities;
  std::vector<VkSurfaceFormatKHR> surfaceFormats;
  std::vector<VkPresentModeKHR> presentModes;
  std::vector<VkQueueFamilyProperties> queueFamilyProperties;
  std::vector<VkExtensionProperties> extensionProperties;
};

struct IceRenderContext
{
  VkAllocationCallbacks* allocator;

  IcePhysicalDeviceInformation gpu;
  VkDevice device;

  u32 graphicsIdx;
  u32 presentIdx;
  u32 transferIdx;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
  VkQueue transferQueue;

  VkCommandPool graphicsCommandPool;
  VkCommandPool transientCommandPool;

  VkExtent2D renderExtent;
  VkRenderPass renderPass;
  std::vector<iceShaderProgram_t> shaderPrograms;
  std::vector<iceShader_t> shaders;

  // Handles all setup for recording a commandBuffer to be executed once
  VkCommandBuffer BeginSingleTimeCommand(VkCommandPool& _pool)
  {
    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = _pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer singleCommand;
    ICE_ASSERT(vkAllocateCommandBuffers(device, &allocInfo, &singleCommand),
               "Failed to create transient command buffer");

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(singleCommand, &beginInfo);

    return singleCommand;
  }

  // Handles the execution and destruction of a commandBuffer
  void EndSingleTimeCommand(VkCommandBuffer _command, VkCommandPool& _pool, VkQueue& _queue)
  {
    vkEndCommandBuffer(_command);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_command;

    vkQueueSubmit(_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_queue);

    vkFreeCommandBuffers(device, _pool, 1, &_command);
  }
};

extern IceRenderContext rContext;

#endif

#endif // !RENDERER_BACKEND_CONTEXT_H
