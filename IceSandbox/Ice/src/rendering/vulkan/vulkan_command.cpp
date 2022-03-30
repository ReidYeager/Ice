
#include "defines.h"
#include "logger.h"

#include "rendering/vulkan/vulkan.h"
#include "rendering/vulkan/vulkan_context.h"

#include <vector>

VkCommandBuffer Ice::RendererVulkan::BeginSingleTimeCommand(VkCommandPool _pool)
{
  // Allocate command =====
  VkCommandBufferAllocateInfo allocInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = _pool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer command;
  IVK_ASSERT(vkAllocateCommandBuffers(context.device, &allocInfo, &command),
             "Failed to allocate sintle time command");

  // Begin command recording =====
  VkCommandBufferBeginInfo beginInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vkBeginCommandBuffer(command, &beginInfo);

  return command;
}

b8 Ice::RendererVulkan::EndSingleTimeCommand(VkCommandBuffer& _command,
                                             VkCommandPool _pool,
                                             VkQueue _queue)
{
  // Complete recording =====
  IVK_ASSERT(vkEndCommandBuffer(_command),
             "Failed to record single-time command buffer");

  // Execution =====
  VkSubmitInfo submitInfo { VK_STRUCTURE_TYPE_SUBMIT_INFO };
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &_command;

  IVK_ASSERT(vkQueueSubmit(_queue, 1, &submitInfo, VK_NULL_HANDLE),
             "Failed to submit single-time command");
  vkQueueWaitIdle(_queue);

  // Destruction =====
  vkFreeCommandBuffers(context.device, _pool, 1, &_command);

  return true;
}

b8 Ice::RendererVulkan::RecordCommandBuffer(u32 _commandIndex, Ice::FrameInformation* _data)
{
  VkCommandBuffer& cmdBuffer = context.commandBuffers[_commandIndex];

  VkCommandBufferBeginInfo beginInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
  beginInfo.flags = 0;

  const u32 clearCount = 6;
  VkClearValue clearValues[clearCount] = {};
  clearValues[0].color = { 0.0f, 0.0f, 0.0f }; // Position
  clearValues[1].color = { 0.0f, 0.0f, 0.0f }; // Normal
  clearValues[2].color = { 0.5f, 0.5f, 0.5f }; // Albedo
  clearValues[3].color = { 0.0f, 0.0f, 0.0f }; // Maps
  clearValues[4].color = { 0.0f, 0.0f, 0.0f }; // Swapchain
  clearValues[5].depthStencil = { 1, 0 }; // Depth

  // ==========
  // Forward
  // ==========
  VkRenderPassBeginInfo forwardBeginInfo { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
  forwardBeginInfo.clearValueCount = 2;
  forwardBeginInfo.pClearValues = &clearValues[4];
  forwardBeginInfo.renderArea.extent = context.swapchainExtent;
  forwardBeginInfo.renderArea.offset = { 0 , 0 };
  forwardBeginInfo.renderPass = context.forward.renderpass;
  forwardBeginInfo.framebuffer = context.forward.framebuffers[_commandIndex];

  // Begin recording =====
  IVK_ASSERT(vkBeginCommandBuffer(cmdBuffer, &beginInfo),
             "Failed to begin command buffer %u", _commandIndex);

  VkDeviceSize zero = 0;

  // Forward pass =====
  {
    vkCmdBeginRenderPass(cmdBuffer, &forwardBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    //vkCmdBindDescriptorSets(cmdBuffer,
    //                        VK_PIPELINE_BIND_POINT_GRAPHICS,
    //                        context.globalPipelinelayout,
    //                        0,
    //                        1,
    //                        &context.globalDescritorSet,
    //                        0,
    //                        nullptr);

    // Materials =====
    for (u32 i = 0; i < _data->materialCount; i++)
    {
      vkCmdBindPipeline(cmdBuffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        _data->materials[i].ivkPipeline);
      vkCmdDraw(cmdBuffer, 6, 1, 0, 0);
    }

    vkCmdEndRenderPass(cmdBuffer);
  }

  // End recording =====
  IVK_ASSERT(vkEndCommandBuffer(cmdBuffer),
             "Failed to record command buffer %u", _commandIndex);

  return true;
}
