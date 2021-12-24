
#include "defines.h"
#include "logger.h"

#include "rendering/vulkan/vulkan_renderer.h"

#include "platform/file_system.h"

u32 IvkRenderer::CreateMesh(const char* _directory)
{
  // Check if exists =====
  u32 i = 0;
  for (const auto& m : meshes)
  {
    if (strcmp(_directory, m.directory) == 0)
    {
      return i;
    }
    i++;
  }

  // Load mesh =====
  IvkMesh mesh = FileSystem::LoadMesh(_directory);
  mesh.directory = _directory;

  CreateBuffer(&mesh.vertBuffer,
               mesh.vertices.size() * sizeof(iceVertex),
               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               (void*)mesh.vertices.data());
  CreateBuffer(&mesh.indexBuffer,
               mesh.indices.size() * sizeof(u32),
               VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               (void*)mesh.indices.data());

  meshes.push_back(mesh);
  return meshes.size() - 1;
}

