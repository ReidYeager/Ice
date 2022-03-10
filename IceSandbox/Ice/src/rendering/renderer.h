
#ifndef ICE_RENDERING_RENDERER_H_
#define ICE_RENDERING_RENDERER_H_

#include "defines.h"

#include "rendering/render_context.h"

namespace Ice {

  struct RendererSettings
  {
    //Ice::Renderer* existingRenderer = nullptr; // Used to setup resources for a new window
    Ice::RenderingApi api;
  };

  class Renderer
  {
  public:
    virtual b8 Init(Ice::RendererSettings _settings) = 0;
    virtual b8 Shutdown() = 0;

  };

}
#endif // !ICE_RENDERING_RENDERER_H_
