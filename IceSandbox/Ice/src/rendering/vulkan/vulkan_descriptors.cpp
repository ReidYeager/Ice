
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
  createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
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
    std::vector<IvkDescriptor> descriptors(7);
    // Global uniform buffer
    descriptors[0].bindingIndex = 0;
    descriptors[0].stageFlags = VK_SHADER_STAGE_ALL;
    descriptors[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // Geo position
    descriptors[1].bindingIndex = 1;
    descriptors[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptors[1].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    // Geo normal
    descriptors[2].bindingIndex = 2;
    descriptors[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptors[2].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    // Geo albedo
    descriptors[3].bindingIndex = 3;
    descriptors[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptors[3].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    // Geo maps
    descriptors[4].bindingIndex = 4;
    descriptors[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptors[4].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    // Geo depth
    descriptors[5].bindingIndex = 5;
    descriptors[5].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptors[5].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    // Shadow depth map
    descriptors[6].bindingIndex = 6;
    descriptors[6].stageFlags = VK_SHADER_STAGE_ALL;
    descriptors[6].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    CreateDescriptorSetAndLayout(descriptors,
                                 &context.globalDescriptorSetLayout,
                                 &context.globalDescritorSet);
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

    IvkGeoBuffer& gb = context.geoBuffer;

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
    write[0].dstSet           = context.globalDescritorSet;
    write[0].dstBinding       = 0;
    write[0].dstArrayElement  = 0;
    write[0].descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write[0].descriptorCount  = 1;
    write[0].pBufferInfo      = &bufferInfo;
    write[0].pImageInfo       = nullptr;
    write[0].pTexelBufferView = nullptr;
    // Geo Position
    write[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write[1].dstSet           = context.globalDescritorSet;
    write[1].dstBinding       = 1;
    write[1].dstArrayElement  = 0;
    write[1].descriptorType   = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    write[1].descriptorCount  = 1;
    write[1].pBufferInfo      = nullptr;
    write[1].pImageInfo       = &geoPositionInfo;
    write[1].pTexelBufferView = nullptr;
    // Geo Normal
    write[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write[2].dstSet           = context.globalDescritorSet;
    write[2].dstBinding       = 2;
    write[2].dstArrayElement  = 0;
    write[2].descriptorType   = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    write[2].descriptorCount  = 1;
    write[2].pBufferInfo      = nullptr;
    write[2].pImageInfo       = &geoNormalInfo;
    write[2].pTexelBufferView = nullptr;
    // Geo Albedo
    write[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write[3].dstSet           = context.globalDescritorSet;
    write[3].dstBinding       = 3;
    write[3].dstArrayElement  = 0;
    write[3].descriptorType   = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    write[3].descriptorCount  = 1;
    write[3].pBufferInfo      = nullptr;
    write[3].pImageInfo       = &geoAlbedoInfo;
    write[3].pTexelBufferView = nullptr;
    // Geo Maps
    write[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write[4].dstSet           = context.globalDescritorSet;
    write[4].dstBinding       = 4;
    write[4].dstArrayElement  = 0;
    write[4].descriptorType   = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    write[4].descriptorCount  = 1;
    write[4].pBufferInfo      = nullptr;
    write[4].pImageInfo       = &geoMapsInfo;
    write[4].pTexelBufferView = nullptr;
    // Geo Depth
    write[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write[5].dstSet           = context.globalDescritorSet;
    write[5].dstBinding       = 5;
    write[5].dstArrayElement  = 0;
    write[5].descriptorType   = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    write[5].descriptorCount  = 1;
    write[5].pBufferInfo      = nullptr;
    write[5].pImageInfo       = &geoDepthInfo;
    write[5].pTexelBufferView = nullptr;
    // Shadow map
    write[6].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write[6].dstSet           = context.globalDescritorSet;
    write[6].dstBinding       = 6;
    write[6].dstArrayElement  = 0;
    write[6].descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write[6].descriptorCount  = 1;
    write[6].pBufferInfo      = nullptr;
    write[6].pImageInfo       = &shadowInfo;
    write[6].pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(context.device, writeCount, write.data(), 0, nullptr);
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
                                    std::vector<IvkDescriptorBinding> _bindings,
                                    u32 _offset)
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
    write.dstBinding = _bindings[i].descriptor.bindingIndex;
    write.dstArrayElement = 0;
    write.descriptorType = _bindings[i].descriptor.type;
    write.descriptorCount = 1;
    write.pTexelBufferView = nullptr;

    switch (_bindings[i].descriptor.type)
    {
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    {
      VkDescriptorBufferInfo info {};
      IvkBuffer* b = &materialBuffers[_bindings[i].data];
      info.buffer = b->buffer;
      info.offset = b->offset;
      info.range = b->size;

      bufferInfos.push_back(info);

      write.pBufferInfo = &bufferInfos[bufferInfos.size() - 1];
      write.pImageInfo = nullptr;
    } break;
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
    {
      VkDescriptorImageInfo info {};
      info.imageLayout = textures[_bindings[i].data].image.layout;
      info.imageView = textures[_bindings[i].data].image.view;
      info.sampler = textures[_bindings[i].data].image.sampler;

      imageInfos.push_back(info);

      write.pBufferInfo = nullptr;
      write.pImageInfo = &imageInfos[imageInfos.size() - 1];
    } break;
    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
    {
      VkDescriptorImageInfo info{};
      info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      info.imageView = textures[_bindings[i].data].image.view;
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
