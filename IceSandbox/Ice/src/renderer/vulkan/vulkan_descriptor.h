
#ifndef ICE_RENDERER_VULKAN_VULKAN_DESCRIPTOR_H_
#define ICE_RENDERER_VULKAN_VULKAN_DESCRIPTOR_H_

#include "defines.h"
#include "renderer/vulkan/vulkan_context.h"

#include <vulkan/vulkan.h>

#include <vector>

VkDescriptorSetLayout CreateSetLayout(IceRenderContext* _rContext,
                                      std::vector<IceShaderBinding> _descriptors);

void CreateDescriptorSet(IceRenderContext* _rContext,
                         VkDescriptorPool const& _pool,
                         VkDescriptorSetLayout const& _layout,
                         VkDescriptorSet& _set);

void BindDescriptors(IceRenderContext* _rContext,
                     VkDescriptorSet& _set,
                     std::vector<IceBuffer> _buffers,
                     std::vector<iceImage_t*> _images);



#endif // !define ICE_RENDERER_VULKAN_VULKAN_DESCRIPTOR_H_
