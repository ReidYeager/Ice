
#include "renderer/renderer.h"
#include "renderer/renderer_backend.h"
#include "renderer/shader_program.h"

Renderer::Renderer()
{
  // Get window surface
  backend = new RendererBackend();
}

Renderer::~Renderer()
{
  
  delete(backend);
}

void Renderer::RenderFrame()
{
  //backend->RenderFrame();
}

u32 Renderer::GetShaderProgram(const char* _name, IceShaderStageFlags _stages)
{
  // Look for an existing shader matching the description
  u32 i = 0;
  for (const auto& sp : shaderPrograms)
  {
    if (sp.stages & _stages && std::strcmp(_name, sp.name) == 0)
    {
      return i;
    }

    i++;
  }

  // Create a new shader
  i = static_cast<u32>(shaderPrograms.size());

  iceShaderProgram_t newShaderProgram{};
  newShaderProgram.name = _name;
  newShaderProgram.stages = _stages;

  if (_stages & ICE_SHADER_STAGE_VERT)
  {
    newShaderProgram.vertIndex = GetShader(_name, ICE_SHADER_STAGE_VERT);
  }

  if (_stages & ICE_SHADER_STAGE_FRAG)
  {
    newShaderProgram.fragIndex = GetShader(_name, ICE_SHADER_STAGE_FRAG);
  }

  if (_stages & ICE_SHADER_STAGE_COMP)
  {
    newShaderProgram.compIndex = GetShader(_name, ICE_SHADER_STAGE_COMP);
  }

  shaderPrograms.push_back(newShaderProgram);
  return i;
}

u32 Renderer::GetShader(const char* _name, IceShaderStageFlags _stage)
{
  // Look for an existing shader matching the description
  u32 i = 0;
  for (const auto& s : shaders)
  {
    if (_stage & s.stage && std::strcmp(_name, s.name) == 0)
    {
      return i;
    }

    i++;
  }

  shaders.push_back(backend->CreateShader(_name, _stage));
}

void Renderer::CreateRenderer()
{
  backend->CreateComponents();
}

void Renderer::CleanupRenderer()
{
  backend->DestroyComponents();
}

void Renderer::RecreateRenderer()
{
  backend->RecreateComponents();
}
