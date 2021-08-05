
#include "renderer/renderer.h"
#include "renderer/renderer_backend.h"
#include "renderer/vulkan/vulkan_backend.h"
#include "renderer/shader_program.h"
#include "platform/file_system.h"

// TODO : Remove all API types & references

// TODO : Add API selection parameter
void IceRenderer::Initialize()
{
  // Get window surface
  backend = new VulkanBackend();
  backend->Initialize();
  LogInfo("Initialized Renderer system");
}

void IceRenderer::Shutdown()
{
  //for (u32 i = 0; i < buffers.size(); i++)
  //{
  //  DestroyBuffer(i);
  //}

  //backend->Shutdown();
  delete(backend);
  LogInfo("Shutdown Renderer system");
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

u32 IceRenderer::GetShaderProgram(IceRenderContext* rContext, const char* _name, IceShaderStageFlags _stages,
                                  std::vector<const char*> _texStrings,
                                  IcePipelineSettingFlags _settings /*= Ice_Pipeline_Default*/)
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
  CreateShaderProgram(rContext, _name, _stages, _texStrings, _settings);
  backend->CreateDescriptorSet(i);

  return i;
}

mesh_t IceRenderer::CreateMesh(const char* _model)
{
  mesh_t m = backend->CreateMesh(_model);
  return m;
}

void IceRenderer::Resize(u32 _width /*= 0*/, u32 _height /*= 0*/)
{
  backend->Resize();
}
