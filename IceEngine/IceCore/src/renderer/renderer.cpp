
#include "renderer/renderer.h"
#include "renderer/renderer_backend.h"
#include "renderer/image.h"
#include "renderer/shader_program.h"
#include "renderer/vulkan/vulkan_backend.h"
#include "platform/file_system.h"
#include "core/ecs_components.h"

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

  for (auto& mes : meshes)
  {
    mes.indexBuffer->FreeBuffer(backend->GetContext());
    mes.vertexBuffer->FreeBuffer(backend->GetContext());
  }

  backend->Shutdown();
  delete(backend);
  LogInfo("Shutdown Renderer system");
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
  std::vector<iceImage_t*> inTextures;
  for (const char* t : _texStrings)
  {
    u32 index = backend->GetTexture(t)->imageIndex;
    inTextures.push_back(backend->GetImage(index));
  }

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
  IceMaterial material = nullptr;
  switch (activeAPI)
  {
  case IceRenderer::Vulkan:
    material = new IvkMaterial_T();
    break;
  default: break;
  }

  material->Initialize(GetContext(),
                       _shaderNames,
                       _shaderStages,
                       ((VulkanBackend*)backend)->GetMVPBuffer());
  material->UpdatePayload(GetContext(), inTextures);
  materials.push_back(material);
  backend->AddMaterial(material);

  return i;
}

u32 IceRenderer::CreateMesh(const char* _model)
{
  u32 i = 0;
  for (const auto& m : meshes)
  {
    if (strcmp(_model, m.directory.c_str()) == 0)
    {
      return i;
    }

    i++;
  }

  mesh_t m = backend->CreateMesh(_model);
  meshes.push_back(m);
  return i;
}

void IceRenderer::Resize(u32 _width /*= 0*/, u32 _height /*= 0*/)
{
  backend->Resize();
}
