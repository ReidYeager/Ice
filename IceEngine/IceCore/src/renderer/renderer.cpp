
#include "renderer/renderer.h"
#include "renderer/renderer_backend.h"
#include "renderer/shader_program.h"
#include "platform/file_system.h"

// TODO : Remove all API types & references

Renderer::Renderer()
{
  // Get window surface
  backend = new RendererBackend();
}

Renderer::~Renderer()
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
}

void Renderer::RecordCommandBuffers()
{
  backend->RecordCommandBuffers();
}

void Renderer::RenderFrame()
{
  // Make draw calls
  // Present render
  backend->RenderFrame();
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

  // Create a new shader program
  i = static_cast<u32>(shaderPrograms.size());

  iceShaderProgram_t newShaderProgram(_name);
  shaderPrograms.push_back(newShaderProgram);

  return i;
}

mesh_t Renderer::CreateMesh(const char* _model)
{
  mesh_t m = FileSystem::LoadMesh(_model);
  //u32 index = CreateBuffer(m.vertices.data(), m.vertices.size() * sizeof(vertex_t),
  //                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  //m.vertexBuffer = buffers[index];
  //index = CreateBuffer(m.indices.data(), m.indices.size() * sizeof(u32),
  //                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
  //m.indexBuffer = buffers[index];

  return m;
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
