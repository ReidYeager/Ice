
#ifndef ICE_RENDERER_MATERIAL_H_
#define ICE_RENDERER_MATERIAL_H_

#include "defines.h"

#include "renderer/shader.h"
#include "renderer/vulkan/vulkan_buffer.h"

#include <vector>

class IceMaterial
{
protected:
  IceShaderInfo info;

public:
  virtual void Initialize(IceRenderContext* _rContext,
                          const std::vector<const char*> _shaderNames,
                          const std::vector<IceShaderStageFlags> _shaderStages,
                          IvkBuffer* _buffer = nullptr) = 0;
  virtual void Shutdown(IceRenderContext* _rContext) = 0;

  // TODO : Replace _images with managed image pointers
  // TODO : Replace IvkBuffer* with an API-agnostic counterpart
  virtual void UpdatePayload(IceRenderContext* _rContext,
                             std::vector<const char*> _images,
                             void* _data,
                             u64 _dataSize) = 0;

  void UpdatePayload(IceRenderContext* _rContext, std::vector<const char*> _images)
  {
    UpdatePayload(_rContext, _images, nullptr, 0);
  }

  void UpdatePayload(IceRenderContext* _rContext, void* _data, u64 _dataSize)
  {
    UpdatePayload(_rContext, {}, _data, _dataSize);
  }
};

#endif // !define ICE_RENDERER_MATERIAL_H_
