
#ifndef ICE_RENDERER_MATERIAL_H_
#define ICE_RENDERER_MATERIAL_H_

#include "defines.h"

#include "renderer/shader.h"
#include "renderer/shader_parameters.h"
#include "renderer/image.h"
#include "renderer/buffer.h"

#include <vector>

class IceMaterial_T
{
protected:
  IceShaderInfo info;
  IceBuffer materialBuffer;

public:
  virtual void Initialize(IceRenderContext* _rContext,
                          const std::vector<const char*> _shaderNames,
                          const std::vector<IceShaderStageFlags> _shaderStages,
                          IceBuffer _buffer = nullptr) = 0;
  virtual void Shutdown(IceRenderContext* _rContext) = 0;

  virtual void UpdateBuffer(IceRenderContext* _rContext,
                            void* _userData,
                            IceShaderBufferParameterFlags _userParameterFlags) = 0;
  virtual void UpdateImages(IceRenderContext* _rContext,
                             std::vector<iceImage_t*> _images,
                             void* _userData,
                             IceShaderBufferParameterFlags _userParameterFlags) = 0;

  void UpdateImages(IceRenderContext* _rContext, std::vector<iceImage_t*> _images)
  {
    UpdateImages(_rContext, _images, nullptr, 0);
  }

  void UpdateImages(IceRenderContext* _rContext, void* _data, u64 _dataSize)
  {
    UpdateImages(_rContext, {}, _data, _dataSize);
  }
};

typedef IceMaterial_T* IceMaterial;

#endif // !define ICE_RENDERER_MATERIAL_H_
