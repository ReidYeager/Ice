
#include "defines.h"
#include "logger.h"

#include "rendering/vulkan/vulkan_renderer.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_vulkan.h"
#include "imgui/imgui_impl_win32.h"


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
  ImGui::Render();

  VkCommandBufferBeginInfo beginInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
  beginInfo.flags = 0;

  VkClearValue clearValues[2] = {};
  clearValues[0].color = { 0.3f, 0.3f, 0.3f };
  clearValues[1].depthStencil = { 1, 0 };

  // Shadow
  VkRenderPassBeginInfo shadowBeginInfo { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
  shadowBeginInfo.clearValueCount = 1;
  shadowBeginInfo.pClearValues = &clearValues[1];
  shadowBeginInfo.renderArea.extent = { shadowResolution, shadowResolution };
  shadowBeginInfo.renderArea.offset = { 0 , 0 };
  shadowBeginInfo.renderPass = shadow.renderpass;
  shadowBeginInfo.framebuffer = shadow.framebuffer;
  // Color
  VkRenderPassBeginInfo colorBeginInfo { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
  colorBeginInfo.clearValueCount = 2;
  colorBeginInfo.pClearValues = clearValues;
  colorBeginInfo.renderArea.extent = context.swapchainExtent;
  colorBeginInfo.renderArea.offset = { 0 , 0 };
  colorBeginInfo.renderPass = context.mainRenderpass;
  colorBeginInfo.framebuffer = context.frameBuffers[_commandIndex];

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

  // Color pass =====
  {
    vkCmdBeginRenderPass(cmdBuffer, &colorBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindDescriptorSets(cmdBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            context.globalPipelinelayout,
                            0,
                            1,
                            &context.globalDescritorSet,
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

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuffer);

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

b8 IvkRenderer::CreateShadowImages()
{
  ICE_ATTEMPT(CreateImage(&shadow.image,
                          { shadowResolution, shadowResolution },
                          VK_FORMAT_D32_SFLOAT,
                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT));
  shadow.image.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR;

  ICE_ATTEMPT(CreateImageView(&shadow.image.view,
                              shadow.image.image,
                              shadow.image.format,
                              VK_IMAGE_ASPECT_DEPTH_BIT));

  ICE_ATTEMPT(CreateImageSampler(&shadow.image));

  return true;
}

b8 IvkRenderer::AddMeshToScene(u32 _meshIndex, u32 _materialIndex)
{
  IvkObject object;
  object.mesh = meshes[_meshIndex];

  float pos = (_meshIndex % 2 == 0) ? -2.0f : 2.0f;
  IceLogDebug("Pos = %u => %f", _meshIndex, pos);

  glm::mat4 tmpTransform = glm::translate(glm::mat4(1), glm::vec3(0, pos, 0));

  CreateBuffer(&object.transformBuffer,
               64,
               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               (void*)&tmpTransform);

  VkDescriptorSetAllocateInfo allocInfo { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
  allocInfo.descriptorPool = context.descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &context.objectDescriptorSetLayout;
  allocInfo.pNext = nullptr;

  IVK_ASSERT(vkAllocateDescriptorSets(context.device,
                                      &allocInfo,
                                      &object.descriptorSet),
             "Failed to allocate object descriptor set");

  VkDescriptorBufferInfo bufferInfo {};
  bufferInfo.buffer = object.transformBuffer.buffer;
  bufferInfo.offset = 0;
  bufferInfo.range = VK_WHOLE_SIZE;

  VkWriteDescriptorSet write { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
  write.dstSet           = object.descriptorSet;
  write.dstBinding       = 0;
  write.dstArrayElement  = 0;
  write.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  write.descriptorCount  = 1;
  write.pBufferInfo      = &bufferInfo;
  write.pImageInfo       = nullptr;
  write.pTexelBufferView = nullptr;
  
  vkUpdateDescriptorSets(context.device, 1, &write, 0, nullptr);

  scene[_materialIndex].push_back(object);

  return true;
}

