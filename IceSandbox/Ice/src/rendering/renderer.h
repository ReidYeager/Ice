
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

    virtual Ice::Shader CreateShader(const Ice::Shader _shader) = 0;
    virtual void DestroyShader(Ice::Shader& _shader) = 0;
    virtual Ice::Material CreateMaterial(MaterialSettings _settings) = 0;
    virtual void DestroyMaterial(Ice::Material& _material) = 0;
  };

}
#endif // !ICE_RENDERING_RENDERER_H_
