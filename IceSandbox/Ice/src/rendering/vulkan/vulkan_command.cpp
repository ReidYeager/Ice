
#include "defines.h"
#include "logger.h"

#include "rendering/vulkan/vulkan_renderer.h"

b8 IvkRenderer::CreateCommandBuffers()
{
  const u32 count = context.swapchainImages.size();
  context.commandsBuffers.resize(count);

  VkCommandBufferAllocateInfo allocInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = count;
  allocInfo.commandPool = context.graphicsCommandPool;

  IVK_ASSERT(vkAllocateCommandBuffers(context.device,
                                      &allocInfo,
                                      context.commandsBuffers.data()),
             "Failed to allocate command buffers");

  return true;
}

b8 IvkRenderer::RecordCommandBuffer(u32 _commandIndex)
{
  VkCommandBufferBeginInfo beginInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
  beginInfo.flags = 0;

  const u32 clearCount = 6;
  VkClearValue clearValues[clearCount] = {};
  clearValues[0].color = { 0.3f, 0.3f, 0.3f }; // Swapchain
  clearValues[1].color = { 0.0f, 0.0f, 0.0f }; // Position
  clearValues[2].color = { 0.0f, 0.0f, 0.0f }; // Normal
  clearValues[3].color = { 0.0f, 0.0f, 0.0f }; // Albedo
  clearValues[4].color = { 0.0f, 0.0f, 0.0f }; // Maps
  clearValues[5].depthStencil = { 1, 0 }; // Depth

  // Shadow
  VkRenderPassBeginInfo shadowBeginInfo { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
  shadowBeginInfo.clearValueCount = 1;
  shadowBeginInfo.pClearValues = &clearValues[5];
  shadowBeginInfo.renderArea.extent = { shadowResolution, shadowResolution };
  shadowBeginInfo.renderArea.offset = { 0 , 0 };
  shadowBeginInfo.renderPass = shadow.renderpass;
  shadowBeginInfo.framebuffer = shadow.framebuffer;
  // Color
  VkRenderPassBeginInfo colorBeginInfo { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
  colorBeginInfo.clearValueCount = clearCount;
  colorBeginInfo.pClearValues = clearValues;
  colorBeginInfo.renderArea.extent = context.swapchainExtent;
  colorBeginInfo.renderArea.offset = { 0 , 0 };
  colorBeginInfo.renderPass = context.deferredRenderpass;
  colorBeginInfo.framebuffer = context.geoBuffers[_commandIndex].framebuffer;

  VkCommandBuffer& cmdBuffer = context.commandsBuffers[_commandIndex];

  // Begin recording =====
  IVK_ASSERT(vkBeginCommandBuffer(cmdBuffer, &beginInfo),
             "Failed to begin command buffer %u", _commandIndex);

  VkDeviceSize zero = 0;

  // Shadow pass =====
  {
    vkCmdBeginRenderPass(cmdBuffer, &shadowBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindDescriptorSets(cmdBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            context.globalPipelinelayout,
                            0,
                            1,
                            &shadow.descriptorSet,
                            0,
                            nullptr);

    // Bind each material =====
    for (u32 matIndex = 0; matIndex < materials.size(); matIndex++)
    {
      vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, materials[matIndex].shadowPipeline);
      vkCmdBindDescriptorSets(cmdBuffer,
                              VK_PIPELINE_BIND_POINT_GRAPHICS,
                              materials[matIndex].pipelineLayout,
                              1,
                              1,
                              &materials[matIndex].descriptorSet,
                              0,
                              nullptr);

      // Draw each object =====
      for (u32 objectIndex = 0; objectIndex < scene[matIndex].size(); objectIndex++)
      {
        IvkObject& object = scene[matIndex][objectIndex];
        vkCmdBindDescriptorSets(cmdBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                materials[matIndex].pipelineLayout,
                                2,
                                1,
                                &object.descriptorSet,
                                0,
                                nullptr);

        vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &object.mesh.vertBuffer.buffer, &zero);
        vkCmdBindIndexBuffer(cmdBuffer, object.mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmdBuffer, object.mesh.indices.size(), 1, 0, 0, 0);
      }
    }
    vkCmdEndRenderPass(cmdBuffer);
  }

  // Deferred pass =====
  {
    vkCmdBeginRenderPass(cmdBuffer, &colorBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindDescriptorSets(cmdBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            context.globalPipelinelayout,
                            0,
                            1,
                            &context.deferredGlobalDescritorSets[_commandIndex],
                            0,
                            nullptr);

    // Bind each material =====
    for (u32 matIndex = 0; matIndex < materials.size(); matIndex++)
    {
      vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, materials[matIndex].pipeline);
      vkCmdBindDescriptorSets(cmdBuffer,
                              VK_PIPELINE_BIND_POINT_GRAPHICS,
                              materials[matIndex].pipelineLayout,
                              1,
                              1,
                              &materials[matIndex].descriptorSet,
                              0,
                              nullptr);

      // Draw each object =====
      for (u32 objectIndex = 0; objectIndex < scene[matIndex].size(); objectIndex++)
      {
        IvkObject& object = scene[matIndex][objectIndex];
        vkCmdBindDescriptorSets(cmdBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                materials[matIndex].pipelineLayout,
                                2,
                                1,
                                &object.descriptorSet,
                                0,
                                nullptr);

        vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &object.mesh.vertBuffer.buffer, &zero);
        vkCmdBindIndexBuffer(cmdBuffer, object.mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmdBuffer, object.mesh.indices.size(), 1, 0, 0, 0);
      }
    }

    // Deferred quad =====
    vkCmdNextSubpass(cmdBuffer, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context.deferredMaterial.pipeline);
    vkCmdBindDescriptorSets(cmdBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            context.deferredMaterial.pipelineLayout,
                            1,
                            1,
                            &context.deferredMaterial.descriptorSet,
                            0,
                            nullptr);
    vkCmdDraw(cmdBuffer, 6, 1, 0, 0);

    vkCmdEndRenderPass(cmdBuffer);
  }


  // End recording =====
  IVK_ASSERT(vkEndCommandBuffer(cmdBuffer),
             "Failed to record command buffer %u", _commandIndex);

  return true;
}

VkCommandBuffer IvkRenderer::BeginSingleTimeCommand(VkCommandPool _pool)
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

b8 IvkRenderer::EndSingleTimeCommand(VkCommandBuffer& _command, VkCommandPool _pool, VkQueue _queue)
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
