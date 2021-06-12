
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
  for (u32 i = 0; i < buffers.size(); i++)
  {
    DestroyBuffer(i);
  }

  for (const auto& sp : shaderPrograms)
  {
    if (sp.stages & ICE_SHADER_STAGE_VERT)
      backend->DestroyShaderModule(shaders[sp.vertIndex].module);
    if (sp.stages & ICE_SHADER_STAGE_FRAG)
      backend->DestroyShaderModule(shaders[sp.fragIndex].module);
    if (sp.stages & ICE_SHADER_STAGE_COMP)
      backend->DestroyShaderModule(shaders[sp.compIndex].module);
  }

  delete(backend);
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

  return i;
}

u32 Renderer::CreateBuffer(const void* _data, VkDeviceSize _size, VkBufferUsageFlags _usage)
{
  u32 index = static_cast<u32>(buffers.size());

  VkBuffer buffer;
  VkDeviceMemory memory;

  backend->CreateAndFillBuffer(buffer, memory, _data, _size, _usage);

  buffers.push_back(buffer);
  bufferMemories.push_back(memory);
  return index;
}

void Renderer::DestroyBuffer(u32 _index)
{
  backend->DestroyBuffer(buffers[_index], bufferMemories[_index]);
}

mesh_t Renderer::CreateMesh(const char* _model)
{
  mesh_t m = FileSystem::LoadMesh(_model);
  u32 index = CreateBuffer(m.vertices.data(), m.vertices.size() * sizeof(vertex_t),
                           VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  m.vertexBuffer = buffers[index];
  index = CreateBuffer(m.indices.data(), m.indices.size() * sizeof(u32),
                       VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
  m.indexBuffer = buffers[index];

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
