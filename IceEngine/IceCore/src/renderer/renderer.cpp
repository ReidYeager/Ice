
#include "renderer/renderer.h"
#include "renderer/renderer_backend.h"
#include "renderer/shader_program.h"
#include "platform/file_system.h"

// TODO : Remove all API types & references

IceRenderer Renderer;

void IceRenderer::Initialize()
{
  // Get window surface
  backend = new RendererBackend();
  IcePrint("Initialized Renderer system");
}

void IceRenderer::Shutdown()
{
  //for (u32 i = 0; i < buffers.size(); i++)
  //{
  //  DestroyBuffer(i);
  //}

  for (const auto& sp : shaderPrograms)
  {
    //sp.Shutdown();

    for (const auto& idx : sp.shaderIndices)
    {
      backend->DestroyShaderModule(shaders[idx].module);
    }
    //if (sp.stages & Ice_Shader_Stage_Vert)
    //  backend->DestroyShaderModule(shaders[sp.vertIndex].module);
    //if (sp.stages & Ice_Shader_Stage_Frag)
    //  backend->DestroyShaderModule(shaders[sp.fragIndex].module);
    //if (sp.stages & Ice_Shader_Stage_Comp)
    //  backend->DestroyShaderModule(shaders[sp.compIndex].module);
  }

  delete(backend);

  IcePrint("Shutdown Renderer system");
}

void IceRenderer::RecordCommandBuffers()
{
  backend->RecordCommandBuffers();
}

void IceRenderer::RenderFrame(IceRenderPacket* _packet)
{
  // Make draw calls
  // Present render
  backend->RenderFrame(_packet);
}

u32 IceRenderer::GetShaderProgram(const char* _name, IceShaderStageFlags _stages)
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

  // Create a new shader program
  i = static_cast<u32>(shaderPrograms.size());

  iceShaderProgram_t newShaderProgram(_name);
  shaderPrograms.push_back(newShaderProgram);

  return i;
}

mesh_t IceRenderer::CreateMesh(const char* _model)
{
  mesh_t m = FileSystem::LoadMesh(_model);
  return m;
}

void IceRenderer::CreateRenderer()
{
  backend->CreateComponents();
}

void IceRenderer::CleanupRenderer()
{
  backend->DestroyComponents();
}

void IceRenderer::RecreateRenderer()
{
  backend->RecreateComponents();
}
