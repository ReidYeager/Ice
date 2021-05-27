
#include "renderer/renderer.h"
#include "renderer/renderer_backend.h"

Renderer::Renderer()
{
  // Get window surface
  backend = new RendererBackend();
}

Renderer::~Renderer()
{
  
  delete(backend);
}

i8 Renderer::RenderFrame()
{
  return 0;
}

i8 Renderer::CreateRenderer()
{
  return 0;
}

i8 Renderer::CleanupRenderer()
{
  return 0;
}

i8 Renderer::RecreateRenderer()
{
  return 0;
}
