
#include "rendering/vulkan/vulkan_renderer.h"


b8 IvkRenderer::CreateDescriptorPool()
{
  // Size definitions =====
  const u32 poolSizeCount = 2;
  VkDescriptorPoolSize sizes[poolSizeCount];

  sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  sizes[0].descriptorCount = 100;

  sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  sizes[1].descriptorCount = 100;

  // Creation =====
  VkDescriptorPoolCreateInfo createInfo { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
  createInfo.flags = 0;
  createInfo.maxSets = 100; // (max possible) 0: Global, 1: per-shader, 2: per-material, 3: per-object
  createInfo.poolSizeCount = poolSizeCount;
  createInfo.pPoolSizes = sizes;

  IVK_ASSERT(vkCreateDescriptorPool(context.device,
                                    &createInfo,
                                    context.alloc,
                                    &context.descriptorPool),
             "Failed to create descriptor pool");

  return true;
}

b8 IvkRenderer::PrepareGlobalDescriptors()
{
  // Prepare buffer =====
  {
    CreateBuffer(&globalDescriptorBuffer,
                 64 + sizeof(IvkLights) + 64,
                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  }

  // Create descriptor set layout =====
  {
    const u32 bindingCount = 7;
    VkDescriptorSetLayoutBinding bindings[bindingCount];
    // Global uniform buffer
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].binding = 0;
    bindings[0].stageFlags = VK_SHADER_STAGE_ALL;
    bindings[0].pImmutableSamplers = nullptr;
    // Geo position
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    bindings[1].descriptorCount = 1;
    bindings[1].binding = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[1].pImmutableSamplers = nullptr;
    // Geo normal
    bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    bindings[2].descriptorCount = 1;
    bindings[2].binding = 2;
    bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[2].pImmutableSamplers = nullptr;
    // Geo albedo
    bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    bindings[3].descriptorCount = 1;
    bindings[3].binding = 3;
    bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[3].pImmutableSamplers = nullptr;
    // Geo maps
    bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    bindings[4].descriptorCount = 1;
    bindings[4].binding = 4;
    bindings[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[4].pImmutableSamplers = nullptr;
    // Geo depth
    bindings[5].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    bindings[5].descriptorCount = 1;
    bindings[5].binding = 5;
    bindings[5].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[5].pImmutableSamplers = nullptr;
    // Shadow depth map
    bindings[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[6].descriptorCount = 1;
    bindings[6].binding = 6;
    bindings[6].stageFlags = VK_SHADER_STAGE_ALL;
    bindings[6].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo createInfo { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    createInfo.flags = 0;
    createInfo.pNext = nullptr;
    createInfo.bindingCount = bindingCount;
    createInfo.pBindings    = bindings;

    IVK_ASSERT(vkCreateDescriptorSetLayout(context.device,
                                           &createInfo,
                                           context.alloc,
                                           &context.globalDescriptorSetLayout),
               "Failed to create global descriptor set layout");
  }

  // Create descriptor set =====
  {
    VkDescriptorSetAllocateInfo allocInfo { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    allocInfo.pNext = nullptr;
    allocInfo.descriptorPool = context.descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &context.globalDescriptorSetLayout;

    context.deferredGlobalDescritorSets.resize(context.swapchainImages.size());

    for (u32 i = 0; i < context.swapchainImages.size(); i++)
    {
      IVK_ASSERT(vkAllocateDescriptorSets(context.device,
                                          &allocInfo,
                                          &context.deferredGlobalDescritorSets[i]),
                 "Failed to allocate global descriptor set");
    }
  }

  // Create pipeline layout =====
  {
    VkPipelineLayoutCreateInfo createInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    createInfo.flags = 0;
    createInfo.pNext = 0;
    createInfo.pushConstantRangeCount = 0;
    createInfo.pPushConstantRanges    = nullptr;
    createInfo.setLayoutCount = 1;
    createInfo.pSetLayouts    = &context.globalDescriptorSetLayout;

    IVK_ASSERT(vkCreatePipelineLayout(context.device,
                                      &createInfo,
                                      context.alloc,
                                      &context.globalPipelinelayout),
               "Failed to create the global render pipeline");
  }

  // Update the global descriptor set =====
  {
    VkDescriptorBufferInfo bufferInfo {};
    bufferInfo.buffer = globalDescriptorBuffer.buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = VK_WHOLE_SIZE;

    VkDescriptorImageInfo shadowInfo {};
    shadowInfo.imageLayout = shadow.image.layout;
    shadowInfo.imageView = shadow.image.view;
    shadowInfo.sampler = shadow.image.sampler;

    const u32 count = context.swapchainImages.size();
    context.deferredGlobalDescritorSets.resize(count);

    for (u32 i = 0; i < count; i++)
    {
      IvkGeoBuffer& gb = context.geoBuffers[i];

      VkDescriptorImageInfo geoPositionInfo {};
      geoPositionInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      geoPositionInfo.imageView = gb.position.view;
      geoPositionInfo.sampler = VK_NULL_HANDLE;

      VkDescriptorImageInfo geoNormalInfo {};
      geoNormalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      geoNormalInfo.imageView = gb.normal.view;
      geoNormalInfo.sampler = VK_NULL_HANDLE;

      VkDescriptorImageInfo geoAlbedoInfo {};
      geoAlbedoInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      geoAlbedoInfo.imageView = gb.albedo.view;
      geoAlbedoInfo.sampler = VK_NULL_HANDLE;

      VkDescriptorImageInfo geoMapsInfo {};
      geoMapsInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      geoMapsInfo.imageView = gb.maps.view;
      geoMapsInfo.sampler = VK_NULL_HANDLE;

      VkDescriptorImageInfo geoDepthInfo {};
      geoDepthInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      geoDepthInfo.imageView = gb.depth.view;
      geoDepthInfo.sampler = VK_NULL_HANDLE;

      const u32 writeCount = 7;
      std::vector<VkWriteDescriptorSet> write(writeCount);
      // Swapchain image
      write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      write[0].dstSet           = context.deferredGlobalDescritorSets[i];
      write[0].dstBinding       = 0;
      write[0].dstArrayElement  = 0;
      write[0].descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      write[0].descriptorCount  = 1;
      write[0].pBufferInfo      = &bufferInfo;
      write[0].pImageInfo       = nullptr;
      write[0].pTexelBufferView = nullptr;
      // Geo Position
      write[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      write[1].dstSet           = context.deferredGlobalDescritorSets[i];
      write[1].dstBinding       = 1;
      write[1].dstArrayElement  = 0;
      write[1].descriptorType   = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
      write[1].descriptorCount  = 1;
      write[1].pBufferInfo      = nullptr;
      write[1].pImageInfo       = &geoPositionInfo;
      write[1].pTexelBufferView = nullptr;
      // Geo Normal
      write[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      write[2].dstSet           = context.deferredGlobalDescritorSets[i];
      write[2].dstBinding       = 2;
      write[2].dstArrayElement  = 0;
      write[2].descriptorType   = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
      write[2].descriptorCount  = 1;
      write[2].pBufferInfo      = nullptr;
      write[2].pImageInfo       = &geoNormalInfo;
      write[2].pTexelBufferView = nullptr;
      // Geo Albedo
      write[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      write[3].dstSet           = context.deferredGlobalDescritorSets[i];
      write[3].dstBinding       = 3;
      write[3].dstArrayElement  = 0;
      write[3].descriptorType   = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
      write[3].descriptorCount  = 1;
      write[3].pBufferInfo      = nullptr;
      write[3].pImageInfo       = &geoAlbedoInfo;
      write[3].pTexelBufferView = nullptr;
      // Geo Maps
      write[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      write[4].dstSet           = context.deferredGlobalDescritorSets[i];
      write[4].dstBinding       = 4;
      write[4].dstArrayElement  = 0;
      write[4].descriptorType   = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
      write[4].descriptorCount  = 1;
      write[4].pBufferInfo      = nullptr;
      write[4].pImageInfo       = &geoMapsInfo;
      write[4].pTexelBufferView = nullptr;
      // Geo Depth
      write[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      write[5].dstSet           = context.deferredGlobalDescritorSets[i];
      write[5].dstBinding       = 5;
      write[5].dstArrayElement  = 0;
      write[5].descriptorType   = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
      write[5].descriptorCount  = 1;
      write[5].pBufferInfo      = nullptr;
      write[5].pImageInfo       = &geoDepthInfo;
      write[5].pTexelBufferView = nullptr;
      // Shadow map
      write[6].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      write[6].dstSet           = context.deferredGlobalDescritorSets[i];
      write[6].dstBinding       = 6;
      write[6].dstArrayElement  = 0;
      write[6].descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      write[6].descriptorCount  = 1;
      write[6].pBufferInfo      = nullptr;
      write[6].pImageInfo       = &shadowInfo;
      write[6].pTexelBufferView = nullptr;

      vkUpdateDescriptorSets(context.device, writeCount, write.data(), 0, nullptr);
    }
  }

  // Scene object descriptor set layout =====
  {
    VkDescriptorSetLayoutBinding binding;
    binding.binding = 0;
    binding.descriptorCount = 1;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding.pImmutableSamplers = nullptr;
    binding.stageFlags = VK_SHADER_STAGE_ALL;

    VkDescriptorSetLayoutCreateInfo createInfo { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    createInfo.flags = 0;
    createInfo.bindingCount = 1;
    createInfo.pBindings = &binding;
    createInfo.pNext = nullptr;

    IVK_ASSERT(vkCreateDescriptorSetLayout(context.device,
                                           &createInfo,
                                           context.alloc,
                                           &context.objectDescriptorSetLayout),
               "Failed to create object descriptor set layout");
  }

  return true;
}

b8 IvkRenderer::UpdateDescriptorSet(VkDescriptorSet& _set,
                                    std::vector<IvkDescriptorBinding> _bindings)
{
  std::vector<VkDescriptorImageInfo> imageInfos;
  imageInfos.reserve(_bindings.size());
  std::vector<VkDescriptorBufferInfo> bufferInfos;
  bufferInfos.reserve(_bindings.size());
  std::vector<VkWriteDescriptorSet> writes;

  for (u32 i = 0; i < _bindings.size(); i++)
  {
    VkWriteDescriptorSet write {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = _set;
    write.dstBinding = i;
    write.dstArrayElement = 0;
    write.descriptorType = _bindings[i].type;
    write.descriptorCount = 1;
    write.pTexelBufferView = nullptr;

    switch (_bindings[i].type)
    {
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    {
      VkDescriptorBufferInfo info {};
      info.buffer = _bindings[i].buffer->buffer;
      info.offset = _bindings[i].buffer->offset;
      info.range = _bindings[i].buffer->size;

      bufferInfos.push_back(info);

      write.pBufferInfo = &bufferInfos[bufferInfos.size() - 1];
      write.pImageInfo = nullptr;
    } break;
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
    {
      VkDescriptorImageInfo info {};
      info.imageLayout = _bindings[i].image->layout;
      info.imageView = _bindings[i].image->view;
      info.sampler = _bindings[i].image->sampler;

      imageInfos.push_back(info);

      write.pBufferInfo = nullptr;
      write.pImageInfo = &imageInfos[imageInfos.size() - 1];
    } break;
    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
    {
      VkDescriptorImageInfo info {};
      info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      info.imageView = _bindings[i].image->view;
      info.sampler = VK_NULL_HANDLE;

      imageInfos.push_back(info);

      write.pBufferInfo = nullptr;
      write.pImageInfo = &imageInfos[imageInfos.size() - 1];
    } break;
    default:
    {
      IceLogError("This descriptor type is not supported");
    } break;
    }

    writes.push_back(write);
  }

  vkUpdateDescriptorSets(context.device, writes.size(), writes.data(), 0, nullptr);

  return true;
}

b8 IvkRenderer::PrepareShadowDescriptors()
{
  // Prepare buffers =====
  {
    CreateBuffer(&shadow.lightMatrixBuffer,
                 64,
                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  }

  // Descriptor set layout =====
  {
    // Currently no need to use a specialized global descriptor for shadows
  }

  // Descriptor set =====
  {
    VkDescriptorSetAllocateInfo allocInfo { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    allocInfo.pNext = nullptr;
    allocInfo.descriptorPool = context.descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &context.globalDescriptorSetLayout;

    IVK_ASSERT(vkAllocateDescriptorSets(context.device,
                                        &allocInfo,
                                        &shadow.descriptorSet),
               "Failed to allocate global descriptor set");
  }

  // Pipeline layout =====
  {
    // Currently no need to use a specialized pipeline layout for shadows
  }

  // Update descriptor set =====
  {
    VkDescriptorBufferInfo bufferInfo {};
    bufferInfo.buffer = shadow.lightMatrixBuffer.buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = VK_WHOLE_SIZE;

    const u32 writeCount = 1;
    std::vector<VkWriteDescriptorSet> write(writeCount);
    write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write[0].dstSet           = shadow.descriptorSet;
    write[0].dstBinding       = 0;
    write[0].dstArrayElement  = 0;
    write[0].descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write[0].descriptorCount  = 1;
    write[0].pBufferInfo      = &bufferInfo;
    write[0].pImageInfo       = nullptr;
    write[0].pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(context.device, writeCount, write.data(), 0, nullptr);
  }

  return true;
}
