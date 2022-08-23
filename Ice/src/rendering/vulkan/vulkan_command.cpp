
#include "defines.h"
#include "rendering/vulkan/vulkan.h"

#include "rendering/vulkan/vulkan_defines.h"

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

  //=========================
  // Begin recording
  //=========================
  IVK_ASSERT(vkBeginCommandBuffer(cmdBuffer, &beginInfo),
             "Failed to begin command buffer %u", _commandIndex);

  // Descriptor sets : 0 = Global, 1 = per-camera, 2 = per-material, 3 = per-object

  //=========================
  // Forward renderpass
  //=========================
  vkCmdBeginRenderPass(cmdBuffer, &forwardBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindDescriptorSets(cmdBuffer,
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          context.globalPipelineLayout,
                          0,
                          1,
                          &context.globalDescriptorSet,
                          0,
                          nullptr);

  // -TODO- : Allow rendering of more than one scene at a time

  u32 camCount = 0;
  Ice::CameraComponent* sceneCameras = _data->cameras->GetArray(&camCount);

  //for (Ice::ECS::ComponentManager<Ice::CameraComponent>* sceneCamerasPtr : _data->sceneCameras)
  for (u32 camIndex = 0; camIndex < camCount; camIndex++)
  {
    vkCmdBindDescriptorSets(cmdBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            context.globalPipelineLayout,
                            1,
                            1,
                            &sceneCameras[camIndex].vulkan.descriptorSet,
                            0,
                            nullptr);

    u32 objectCount = 0;
    Ice::RenderComponent* sceneObjects = _data->renderables->GetArray(&objectCount);

    for (u32 objectIndex = 0; objectIndex < objectCount; objectIndex++)
    {
      vkCmdBindPipeline(cmdBuffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        sceneObjects[objectIndex].material->vulkan.pipeline);

      vkCmdBindDescriptorSets(cmdBuffer,
                              VK_PIPELINE_BIND_POINT_GRAPHICS,
                              sceneObjects[objectIndex].material->vulkan.pipelineLayout,
                              2,
                              1,
                              &sceneObjects[objectIndex].material->vulkan.descriptorSet,
                              0,
                              nullptr);

      vkCmdBindDescriptorSets(cmdBuffer,
                              VK_PIPELINE_BIND_POINT_GRAPHICS,
                              sceneObjects[objectIndex].material->vulkan.pipelineLayout,
                              3,
                              1,
                              &sceneObjects[objectIndex].vulkan.descriptorSet,
                              0,
                              nullptr);

      vkCmdBindVertexBuffers(cmdBuffer,
                             0,
                             1,
                             &sceneObjects[objectIndex].mesh->vertexBuffer.buffer->vulkan.buffer,
                             &sceneObjects[objectIndex].mesh->vertexBuffer.offset);
      vkCmdBindIndexBuffer(cmdBuffer,
                           sceneObjects[objectIndex].mesh->indexBuffer.buffer->vulkan.buffer,
                           sceneObjects[objectIndex].mesh->indexBuffer.offset,
                           VK_INDEX_TYPE_UINT32);

      // TODO : Instanced rendering -- DrawIndexed can use a significant amount of time
      vkCmdDrawIndexed(cmdBuffer, sceneObjects[objectIndex].mesh->indexCount, 1, 0, 0, 0);
    }
  }
  vkCmdEndRenderPass(cmdBuffer);

  // End recording =====
  IVK_ASSERT(vkEndCommandBuffer(cmdBuffer),
             "Failed to record command buffer %u", _commandIndex);

  return true;
}
