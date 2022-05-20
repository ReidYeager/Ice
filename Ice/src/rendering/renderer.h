
#ifndef ICE_RENDERING_RENDERER_H_
#define ICE_RENDERING_RENDERER_H_

#include "defines.h"

#include "rendering/renderer_defines.h"

#include <vector>
#include <string>

namespace Ice {

  class Renderer
  {
  public:
    virtual ~Renderer() {}

    virtual b8 Init(Ice::RendererSettings _settings, const char* _title, u32 _version) = 0;
    // NOTE : Given the frequency this is called, should use something more efficient than virtual.
    virtual b8 RenderFrame(FrameInformation* _data) = 0;
    virtual b8 Shutdown() = 0;

    virtual b8 CreateShader(Ice::Shader* _shader) = 0;
    virtual void DestroyShader(Ice::Shader& _shader) = 0;
    virtual b8 CreateMaterial(Ice::Material* _material) = 0;
    virtual void DestroyMaterial(Ice::Material& _material) = 0;

    virtual b8 SetMaterialInput(Ice::Material* _material, u32 _bindIndex, Ice::Image* _image) = 0;
    //virtual b8 SetMaterialInput(u32 _bindIndex, Ice::Buffer* _buffer);

    virtual b8 CreateTexture(Ice::Image* _image, void* _data) = 0;
    virtual void DestroyImage(Ice::Image* _image) = 0;

    virtual b8 CreateBufferMemory(Ice::Buffer* _outBuffer,
                                  u64 _elementSize,
                                  u32 _elementCount,
                                  Ice::BufferMemoryUsageFlags _usage) = 0;
    virtual void DestroyBufferMemory(Ice::Buffer* _buffer) = 0;
    virtual b8 PushDataToBuffer(void* _data, const Ice::BufferSegment _segmentInfo) = 0;

    virtual b8 InitializeRenderComponent(Ice::RenderComponent* _component,
                                         Ice::BufferSegment* _TransformBuffer) = 0;
    virtual b8 InitializeCamera(Ice::CameraComponent* _camera,
                                Ice::BufferSegment _transformSegment,
                                Ice::CameraSettings _settings) = 0;
  };

}

extern Ice::Renderer* renderer;

#endif // !ICE_RENDERING_RENDERER_H_
