
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
  std::vector<IceBuffer> shaderBuffers;

public:
  virtual void Initialize(IceRenderContext* _rContext,
                          const std::vector<const char*> _shaderNames,
                          const std::vector<IceShaderStageFlags> _shaderStages,
                          std::vector<iceImage_t*> _images = {}) = 0;
  virtual void Shutdown(IceRenderContext* _rContext) = 0;

  virtual void UpdateBuffer(IceRenderContext* _rContext,
                            IceShaderStageFlags _stage,
                            IceShaderBufferParameterFlags _userParameterFlags,
                            void* _userData) = 0;
  virtual void UpdateImages(IceRenderContext* _rContext,
                            std::vector<iceImage_t*> _images) = 0;
};

typedef IceMaterial_T* IceMaterial;

#endif // !define ICE_RENDERER_MATERIAL_H_
