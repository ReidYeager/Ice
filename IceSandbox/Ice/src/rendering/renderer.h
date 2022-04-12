
#ifndef ICE_RENDERING_RENDERER_H_
#define ICE_RENDERING_RENDERER_H_

#include "defines.h"

#include "rendering/renderer_context.h"

#include <vector>
#include <string>

namespace Ice {

  class Renderer
  {
  public:
    virtual b8 Init(Ice::RendererSettings _settings) = 0;
    // NOTE : Given the frequency this is called, I should find a more efficient way to handle its polymorphism
    virtual b8 RenderFrame(FrameInformation* _data) = 0;
    virtual b8 Shutdown() = 0;

    virtual b8 CreateShader(Ice::Shader* _shader) = 0;
    virtual void DestroyShader(Ice::Shader& _shader) = 0;
    virtual b8 CreateMaterial(Ice::Material* _material) = 0;
    virtual void DestroyMaterial(Ice::Material& _material) = 0;

    virtual b8 CreateBufferMemory(Ice::Buffer* _outBuffer,
                                  u64 _size,
                                  Ice::BufferMemoryUsageFlags _usage) = 0;
    virtual void DestroyBufferMemory(Ice::Buffer* _buffer) = 0;
    virtual b8 PushDataToBuffer(void* _data,
                                const Ice::Buffer* _buffer,
                                const Ice::BufferSegment _segmentInfo) = 0;

    virtual b8 InitializeRenderComponent(Ice::RenderComponent* _component,
                                         Ice::Buffer* _TransformBuffer) = 0;
  };

}
#endif // !ICE_RENDERING_RENDERER_H_
