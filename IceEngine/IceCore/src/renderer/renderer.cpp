
#include "renderer/renderer.h"
#include "renderer/renderer_backend.h"
#include "renderer/vulkan/vulkan_backend.h"
#include "renderer/shader_program.h"
#include "platform/file_system.h"

// TODO : Remove all API types & references

void IceRenderer::Initialize(IceRenderingAPI _api)
{
  backend = new VulkanBackend();
  backend->Initialize();
  activeAPI = Vulkan;
  LogInfo("Initialized Renderer system");
}

void IceRenderer::Shutdown()
{
  for (auto& m : materials)
  {
    m->Shutdown(backend->GetContext());
    free(m);
  }

  backend->Shutdown();
  delete(backend);
  LogInfo("Shutdown Renderer system");
}

void IceRenderer::RecordCommandBuffers(u32 _shaderIndex)
{
  backend->RecordCommandBuffers((IvkMaterial*)materials[_shaderIndex]);
}

void IceRenderer::RenderFrame(IceRenderPacket* _packet)
{
  // Make draw calls
  // Present render
  backend->RenderFrame(_packet);
}

u32 IceRenderer::GetMaterial(std::vector<const char*> _shaderNames,
                             std::vector<IceShaderStageFlags> _shaderStages,
                             std::vector<const char*> _texStrings,
                             IceFlag _renderSettings /*= 0*/)
{
  // Look for an existing shader matching the description
  u32 i = 0;
  // TODO : Make IceMaterials identifiable
  //for (const auto& sp : materials)
  //{
  //  if ()
  //  {
  //    return i;
  //  }
  //  i++;
  //}

  // Create a new shader program
  i = static_cast<u32>(materials.size());
  IceMaterial* material = nullptr;
  switch (activeAPI)
  {
  case IceRenderer::Vulkan:
    material = new IvkMaterial();
    break;
  default: break;
  }

  material->Initialize(GetContext(),
                       _shaderNames,
                       _shaderStages,
                       ((VulkanBackend*)backend)->GetMVPBuffer());
  material->UpdatePayload(GetContext(), _texStrings);
  materials.push_back(material);

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
