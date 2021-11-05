
#ifndef ICE_RENDERER_VULKAN_VULKAN_MATERIAL_H_
#define ICE_RENDERER_VULKAN_VULKAN_MATERIAL_H_

#include "defines.h"

#include "renderer/vulkan/vulkan_context.h"
#include "renderer/vulkan/vulkan_shader.h"
#include "renderer/buffer.h"
#include "renderer/image.h"
#include "renderer/material.h"
#include "renderer/shader_parameters.h"

#include <vector>
#include <vulkan/vulkan.h>

class IvkMaterial_T : public IceMaterial_T
{
private:
  VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
  std::vector<VkDescriptorSetLayout> dSetLayouts;
  std::vector<VkDescriptorSet> descriptorSets;
  VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
  VkPipeline pipeline = VK_NULL_HANDLE;

  IceShaderBufferParameterFlags materialUniformInputs;

  std::vector<IvkShader> shaders;

public:
  // Initializes all components required to use in rendering
  void Initialize(IceRenderContext* _rContext,
                  const std::vector<const char*> _shaderNames,
                  const std::vector<IceShaderStageFlags> _shaderStages,
                  std::vector<iceImage_t*> _images = {}) override;
  // Destroys its rendering components
  void Shutdown(IceRenderContext* _rContext) override;
  // Pipeline components separated as they may change at runtime
  void DestroyPipelineComponents(IceRenderContext* _rContext);
  // Pipeline components separated as they may change at runtime
  void CreatePipelineComponents(IceRenderContext* _rContext);
  // Reloads shader source code if changes were made since loading
  void UpdateSources(IceRenderContext* _rContext);

  void UpdateBuffer(IceRenderContext* _rContext,
                    IceShaderStageFlags _stage,
                    IceShaderBufferParameterFlags _userParameterFlags,
                    void* _userData) override;
  void UpdateImages(IceRenderContext* _rContext,
                    std::vector<iceImage_t*> _images) override; 
  // Binds the data and images to update the descriptor set
  void UpdateDescriptors(IceRenderContext* _rContext,
                         std::vector<iceImage_t*> _images);
  // Records commands on how to render this material
  void Render(VkCommandBuffer& _command, const void* _modelMatrix, const void* _viewProjMatrix);

private:
  // Loads all the material's shaders
  std::vector<IvkShader> GetShaders(IceRenderContext* _rContext);
  // Gets the shader's module and bindings
  IvkShader LoadShader(IceRenderContext* _rContext, const char* _name, IceShaderStageFlags _stage);
  // Loads the given shader and creates its shader module
  void CreateShaderModule(IceRenderContext* _rContext, IvkShader& _shader, const char* _directory);
  // Processes the shader's layout file for bindings
  void FillShaderBindings(IvkShader& _shader, const char* _directory);

  void CreateDescriptorSetLayout(IceRenderContext* _rContext,
                                 const std::vector<IvkShader>& _shaders);
  void CreateDescriptorSets(IceRenderContext* _rContext);
  void CreatePipelineLayout(IceRenderContext* _rContext);
  void CreatePipeline(IceRenderContext* _rContext, std::vector<IvkShader> _shaders);

};

#endif // !ICE_RENDERER_VULKAN_VULKAN_SHADER_H_
