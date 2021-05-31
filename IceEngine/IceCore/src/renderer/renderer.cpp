
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
  backend->CreateComponents();
  return 0;
}

i8 Renderer::CleanupRenderer()
{
  backend->DestroyComponents();
  return 0;
}

i8 Renderer::RecreateRenderer()
{
  backend->RecreateComponents();
  return 0;
}
