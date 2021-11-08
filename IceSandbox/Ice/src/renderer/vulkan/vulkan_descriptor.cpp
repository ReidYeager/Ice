
#include "defines.h"
#include "logger.h"

#include "renderer/vulkan/vulkan_backend.h"
#include "renderer/image.h"
#include "renderer/vulkan/vulkan_buffer.h"

#include <vulkan/vulkan.h>

#include <vector>

VkDescriptorSetLayout CreateSetLayout(IceRenderContext* _rContext,
                                      std::vector<IceShaderBinding> _descriptors)
{
  std::vector<VkDescriptorSetLayoutBinding> bindings(_descriptors.size());

  VkDescriptorSetLayoutBinding descriptor;
  descriptor.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
  descriptor.descriptorCount = 1;
  descriptor.pImmutableSamplers = nullptr;

  u32 i = 0;

  for (IceShaderBinding d : _descriptors)
  {
    switch (d)
    {
    case Ice_Shader_Binding_Buffer:
      descriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      break;
    case Ice_Shader_Binding_Image:
      descriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      break;
    case Ice_Shader_Binding_PushConstant:
    case Ice_Shader_Binding_Count:
    case Ice_Shader_Binding_Invalid:
    default:
      continue;
    }

    descriptor.binding = i;
    bindings[i] = descriptor;
    i++;
  }

  VkDescriptorSetLayoutCreateInfo createInfo {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
  createInfo.bindingCount = bindings.size();
  createInfo.pBindings = bindings.data();
  createInfo.flags = 0;

  VkDescriptorSetLayout tmpLayout;

  IVK_ASSERT(vkCreateDescriptorSetLayout(_rContext->device,
                                         &createInfo,
                                         _rContext->allocator,
                                         &tmpLayout),
             "Failed to create descriptor set layout");

  return tmpLayout;
}

void CreateDescriptorSet(IceRenderContext* _rContext,
                         VkDescriptorPool const& _pool,
                         VkDescriptorSetLayout const& _layout,
                         VkDescriptorSet& _set)
{
  VkDescriptorSetAllocateInfo allocInfo {};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = _pool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &_layout;

  IVK_ASSERT(vkAllocateDescriptorSets(_rContext->device, &allocInfo, &_set),
             "Failed to allocate descriptor set");
}

void BindDescriptors(IceRenderContext* _rContext,
                     VkDescriptorSet& _set,
                     std::vector<IceBuffer> _buffers,
                     std::vector<iceImage_t*> _images)
{
  
}
