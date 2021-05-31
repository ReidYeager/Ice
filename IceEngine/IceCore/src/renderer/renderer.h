
#ifndef RENDERER_RENDERER_H
#define RENDERER_RENDERER_H 1
// TODO : Define API agnostic calls

#include "defines.h"
#include "renderer/renderer_backend.h"

class Renderer
{
//=================================================
// Variables
//=================================================
private:
  RendererBackend* backend = nullptr;

//=================================================
// Functions
//=================================================
public:
  Renderer();
  ~Renderer();

  i8 RenderFrame();

private:
  i8 Initialize();
  i8 Shutdown();

  i8 CreateRenderer();
  i8 CleanupRenderer();
  i8 RecreateRenderer();

};

#endif // !RENDERER_RENDERER_H
