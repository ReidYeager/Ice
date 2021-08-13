
#ifndef ICE_RENDERER_VULKAN_VULKAN_MATERIAL_H_
#define ICE_RENDERER_VULKAN_VULKAN_MATERIAL_H_

#include "defines.h"

#include "renderer/vulkan/vulkan_context.h"
#include "renderer/vulkan/vulkan_shader.h"
#include "renderer/shader.h"

#include <vulkan/vulkan.h>
#include <vector>

class IvkMaterial
{
private:
  IceShaderInfo info;

  // TODO : Replace with handled buffer?
  IvkBuffer* buffer;

  // TODO : Add pipeline settings
  VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
  VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
  VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
  VkPipeline pipeline = VK_NULL_HANDLE;

public:
  IvkMaterial(IceRenderContext* _rContext,
            const std::vector<const char*> _shaderNames,
            const std::vector<IceShaderStageFlags> _shaderStages);
  void Shutdown(IceRenderContext* _rContext);

  // TODO : Replace _images with managed image pointers
  void UpdatePayload(IceRenderContext* _rContext,
                     std::vector<const char*> _images,
                     IvkBuffer* _buffer = nullptr);
  void Render(VkCommandBuffer& _command);

private:
  std::vector<IvkShader> GetShaders(
      IceRenderContext* _rContext, const std::vector<IceShaderStageFlags> _shaderStages);
  IvkShader LoadShader(
      IceRenderContext* _rContext, const char* _name, IceShaderStageFlags _stage);
  void CreateShaderModule(IceRenderContext* _rContext, IvkShader& _shader, const char* _directory);
  void FillShaderBindings(IvkShader& _shader, const char* _directory);
  void CreateDescriptorSetLayout(
      IceRenderContext* _rContext, const std::vector<IvkShader>& _shaders);
  void CreateDescriptorSet(IceRenderContext* _rContext);
  void CreatePipelineLayout(IceRenderContext* _rContext);
  void CreatePipeline(IceRenderContext* _rContext, std::vector<IvkShader> _shaders);

};

#endif // !ICE_RENDERER_VULKAN_VULKAN_SHADER_H_
