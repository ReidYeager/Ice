
#include "defines.h"

#include "core/application.h"

#include "core/input.h"
#include "core/ecs/ecs.h"
#include "core/platform/platform.h"
#include "rendering/vulkan/vulkan.h"
#include "math/linear.h"
#include "math/transform.h"
#include "core/ecs/entity.h"
#include "tools/array.h"
#include "tools/pool.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>

Ice::ApplicationSettings appSettings;
Ice::RendererVulkan* renderer;
b8 isRunning;

// Rendering =====
Ice::CompactPool<Ice::Shader> shaders;
Ice::CompactPool<Ice::Material> materials;
Ice::CompactPool<Ice::MaterialSettings> materialSettings;
Ice::CompactPool<Ice::MeshInformation> meshes;
Ice::CompactPool<Ice::Image> textures;

Ice::FrameInformation frameInfo{};

Ice::Buffer transformsBuffer;

//=========================
// Time
//=========================

Ice::TimeStruct Ice::time;
// Setup time =====
std::chrono::steady_clock::time_point realtimeStart, frameStart, frameEnd;
const float microToSecond = 0.000001f;

void InitTime()
{
  Ice::time.deltaTime = 0.0f;
  Ice::time.timeSinceStart = 0.0f;
  Ice::time.frameCount = 0;

  realtimeStart = std::chrono::steady_clock::now();
  frameStart = frameEnd = realtimeStart;

  Ice::time.deltaTime = 0.0f;
}

void UpdateTime()
{
  frameEnd = std::chrono::steady_clock::now();

  Ice::time.deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - frameStart).count() * microToSecond;
  Ice::time.timeSinceStart = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - realtimeStart).count() * microToSecond;

  Ice::time.frameCount++;

  frameStart = frameEnd;
}

//=========================
// Application
//=========================

// TODO : Hard-code default texture information into engine so it doesn't have to load anything
Ice::Image defaultTexture;

b8 IceApplicationInitialize(Ice::ApplicationSettings _settings)
{
  InitTime();
  appSettings = _settings;

  // Platform =====
  if (!Ice::platform.CreateNewWindow(_settings.window))
  {
    IceLogFatal("Failed to initialize the renderer");
    return false;
  }
  Ice::input.Initialize();

  // Rendering =====
  switch (_settings.rendererCore.api)
  {
  case Ice::RenderApiVulkan:
  {
    renderer = new Ice::RendererVulkan();
  } break;
  default:
  {
    IceLogFatal("Selected rendering API not supported");
    return false;
  } break;
  }

  if (!renderer->Init(_settings.rendererCore, _settings.window.title, _settings.version))
  {
    IceLogFatal("Failed to initialize the renderer");
    return false;
  }

  //shaders = (Ice::Shader*)Ice::MemoryAllocZero(
  //  sizeof(Ice::Shader) * _settings.maxShaderCount);
  shaders.Resize(_settings.maxShaderCount);
  materials.Resize(_settings.maxMaterialCount);
  materialSettings.Resize(_settings.maxMaterialCount);
  meshes.Resize(_settings.maxMeshCount);
  textures.Resize(_settings.maxTextureCount);

  Ice::GetComponentArray<Ice::Transform>().Resize(16);
  ICE_ATTEMPT(renderer->CreateBufferMemory(
              &transformsBuffer,
              sizeof(Ice::mat4),
              Ice::GetComponentArray<Ice::Transform>().GetAllocatedSize(),
              Ice::Buffer_Memory_Shader_Read
              | Ice::Buffer_Memory_Transfer_Src
              | Ice::Buffer_Memory_Transfer_Dst));

  // Initialize transforms =====
  Ice::Transform* transforms = Ice::GetComponentArray<Ice::Transform>().GetArray();
  u32 transformCount = Ice::GetComponentArray<Ice::Transform>().GetAllocatedSize();
  for (u32 i = 0; i < transformCount; i++)
  {
    transforms[i] = Ice::Transform();
  }

  // Game =====
  ICE_ATTEMPT(_settings.GameInit());

  UpdateTime();
  IceLogInfo("} Startup %2.3f s", Ice::time.timeSinceStart);

  return true;
}

b8 IceApplicationUpdate()
{
  Ice::input.Update();
  UpdateTime();

  Ice::mat4 globalDescriptorData;

  Ice::FrameInformation frameInfo{};
  frameInfo.cameras = &Ice::GetComponentArray<Ice::CameraComponent>();
  frameInfo.renderables = &Ice::GetComponentArray<Ice::RenderComponent>();
  frameInfo.meshes = &meshes;
  frameInfo.materials = &materials;

  while (isRunning && Ice::platform.Update())
  {
    ICE_ATTEMPT(appSettings.GameUpdate(Ice::time.deltaTime));

    globalDescriptorData[0] = Ice::time.timeSinceStart;
    renderer->PushDataToGlobalDescriptors(&globalDescriptorData);

    ICE_ATTEMPT(Ice::UpdateTransforms());

    ICE_ATTEMPT(renderer->RenderFrame(&frameInfo));

    Ice::input.Update();
    UpdateTime();
  }

  return true;
}

b8 IceApplicationShutdown()
{
  ICE_ATTEMPT(appSettings.GameShutdown());

  for (u32 i = 0; i < textures.Size(); i++)
  {
    renderer->DestroyImage(&textures[i]);
  }
  textures.Shutdown();

  for (Ice::MeshInformation& m : meshes)
  {
    renderer->DestroyBufferMemory(&m.mesh.buffer);
  }
  meshes.Shutdown();

  for (Ice::Material& m : materials)
  {
    renderer->DestroyMaterial(m);
  }
  materials.Shutdown();
  materialSettings.Shutdown();

  for (u32 i = 0; i < shaders.Size(); i++)
  {
    renderer->DestroyShader(shaders[i]);
  }
  shaders.Shutdown();

  for (Ice::Entity& e : Ice::SceneView<Ice::CameraComponent>())
  {
    renderer->DestroyBufferMemory(&e.GetComponent<Ice::CameraComponent>()->buffer);
  }

  renderer->DestroyBufferMemory(&transformsBuffer);

  renderer->Shutdown();
  delete(renderer);

  Ice::input.Shutdown();
  Ice::CloseWindow();
  Ice::platform.Shutdown();

  return true;
}

u32 Ice::CreateVersion(u8 _major, u8 _minor, u8 _patch)
{
  return ((u32)_major << 16) | ((u32)_minor << 8) | ((u32)_patch);
}

u32 Ice::Run(ApplicationSettings _settings)
{
  if (!IceApplicationInitialize(_settings))
  {
    IceLogFatal("Ice application initialization failed");
    return -1;
  }

  isRunning = true;

  if (!IceApplicationUpdate())
  {
    IceLogFatal("Ice application update failed");
    return -2;
  }

  isRunning = false;

  if (!IceApplicationShutdown())
  {
    IceLogFatal("Ice application shutdown failed");
    return -3;
  }

  return 0;
}

void Ice::Shutdown()
{
  isRunning = false;
}

void Ice::CloseWindow()
{
  platform.CloseWindow();
}

//=========================
// Rendering
//=========================

struct HashedVertex
{
  glm::vec3 position;
  glm::vec2 uv;
  glm::vec3 normal;

  bool operator==(const HashedVertex& other) const
  {
    return position == other.position && normal == other.normal && uv == other.uv;
  }
};

// Used to map vertices into an unordered array during mesh building
namespace std {
template<> struct hash<HashedVertex>
{
  size_t operator()(HashedVertex const& vertex) const
  {
    return ((hash<glm::vec3>()(vertex.position) ^
             (hash<glm::vec2>()(vertex.uv) << 1)) >> 1) ^
      (hash<glm::vec3>()(vertex.normal) << 1);
  }
};
}

b8 CreateMesh(const char* _directory, Ice::Mesh* _mesh)
{
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  // Load mesh =====
  {
    std::string loadWarnings, loadErrors;

    if (!tinyobj::LoadObj(&attrib,
                          &shapes,
                          &materials,
                          &loadWarnings,
                          &loadErrors,
                          _directory))
    {
      IceLogError("Failed to load mesh '%s'\n> tinyobj warnings: '%s'\n> tinyobj errors: '%s'",
                  _directory,
                  loadWarnings.c_str(),
                  loadErrors.c_str());
    }
  }

  std::vector<Ice::Vertex> vertices;
  std::vector<u32> indices;

  // Assemble mesh =====
  std::unordered_map<HashedVertex, u32> vertMap = {};
  {
    HashedVertex vert{};
    for (const auto& shape : shapes)
    {
      for (const auto& index : shape.mesh.indices)
      {
        vert.position = {
          attrib.vertices[3 * index.vertex_index + 0],
          attrib.vertices[3 * index.vertex_index + 1],
          attrib.vertices[3 * index.vertex_index + 2]
        };

        vert.uv = {
          attrib.texcoords[2 * index.texcoord_index + 0],
          1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
        };

        vert.normal = {
          attrib.normals[3 * index.normal_index + 0],
          attrib.normals[3 * index.normal_index + 1],
          attrib.normals[3 * index.normal_index + 2]
        };

        if (vertMap.count(vert) == 0)
        {
          vertMap[vert] = (u32)vertices.size();

          Ice::Vertex properVert{};
          properVert.position = { vert.position.x, vert.position.y, vert.position.z };
          properVert.uv = { vert.uv.x, vert.uv.y };
          properVert.normal = { vert.normal.x, vert.normal.y, vert.normal.z };

          vertices.push_back(properVert);
        }
        indices.push_back(vertMap[vert]);
      }
    }
  }

  // Create mesh =====
  {
    Ice::Mesh& newMesh = *_mesh;

    newMesh.indexCount = (u32)indices.size();
    ICE_ATTEMPT(renderer->CreateBufferMemory(&newMesh.buffer,
                                             (sizeof(Ice::Vertex) * vertices.size() +
                                              (sizeof(u32) * indices.size())),
                                             1,
                                             Ice::Buffer_Memory_Vertex | Ice::Buffer_Memory_Index));

    newMesh.vertexBuffer.buffer = &newMesh.buffer;
    newMesh.vertexBuffer.offset = 0;
    newMesh.vertexBuffer.elementSize = sizeof(Ice::Vertex) * vertices.size();
    newMesh.vertexBuffer.count = 1;
    if (!renderer->PushDataToBuffer(vertices.data(), newMesh.vertexBuffer))
    {
      IceLogError("Failed to push vertex data to its buffer");
      renderer->DestroyBufferMemory(&newMesh.buffer);
      return false;
    }

    newMesh.indexBuffer.buffer = &newMesh.buffer;
    newMesh.indexBuffer.offset = newMesh.vertexBuffer.elementSize;
    newMesh.indexBuffer.elementSize = sizeof(u32) * indices.size();
    newMesh.indexBuffer.count = 1;
    if (!renderer->PushDataToBuffer(indices.data(), newMesh.indexBuffer))
    {
      IceLogError("Failed to push index data to its buffer");
      renderer->DestroyBufferMemory(&newMesh.buffer);
      return false;
    }
  }

  return true;
}

b8 Ice::GetMesh(const char* _directory, u32* _mesh)
{
  u32 meshIndex = 0;
  for (Ice::MeshInformation m : meshes)
  {
    if (m.fileName.compare(_directory) == 0)
    {
      *_mesh = meshIndex;
      return true;
    }
    meshIndex++;
  }

  Ice::MeshInformation& m = meshes.GetNewElement(_mesh);
  if (!CreateMesh(_directory, &m.mesh))
  {
    meshes.ReturnElement(*_mesh);
    *_mesh = Ice::null32;
    return false;
  }

  m.fileName = _directory;
  return true;
}

b8 Ice::CreateMaterial(Ice::MaterialSettings _settings, u32* _material /*= nullptr*/)
{
  // Don't search for existing materials to allow one material setup with multiple buffer values
  // TODO : Create buffers for multiple instances of a material (instead of creating multiple materials)

  if (materials.Size() == appSettings.maxMaterialCount)
  {
    IceLogError("Maximum material count (%u) reached", appSettings.maxMaterialCount);
    return false;
  }

  // Get shaders' info =====
  b8 shaderFound = false;
  std::vector<u32> shaderIndices;

  for (u32 i = 0; i < _settings.shaderSettings.size(); i++)
  {
    shaderFound = false;
    u32 index = 0;

    // Check for existing shader
    for (Ice::Shader& oldShader : shaders)
    {
      std::string oldNameWithoutExt = oldShader.settings.fileDirectory.
                                      substr(0, oldShader.settings.fileDirectory.size() - 5);
      std::string newName= _settings.shaderSettings[i].fileDirectory;

      if (oldNameWithoutExt.compare(newName) == 0
          && _settings.shaderSettings[i].type == oldShader.settings.type)
      {
        shaderFound = true;
        shaderIndices.push_back(index);
        index++;
        break;
      }
      index++;
    }

    // Create new shader
    if (!shaderFound)
    {
      if (shaders.Size() == appSettings.maxShaderCount)
      {
        IceLogError("Maximum shader count (%u) reached", appSettings.maxShaderCount);
        return false;
      }

      Ice::Shader& newshader = shaders.GetNewElement();
      if (!renderer->CreateShader(_settings.shaderSettings[i], &newshader))
      {
        IceLogError("Failed to create a shader\n> '%s'", newshader.settings.fileDirectory.c_str());
        return false;
      }

      shaderIndices.push_back(index);
      index++;
    }
  }

  std::vector<Ice::Shader*> shaderPointers;
  for (u32 i : shaderIndices)
  {
    shaderPointers.push_back(&shaders[i]);
  }

  materialSettings.GetNewElement() = _settings;

  // Create material =====
  Ice::Material& newmaterial = materials.GetNewElement(_material);
  newmaterial.subpassIndex = _settings.subpassIndex;
  newmaterial.shaders = shaderPointers;
  if (!renderer->CreateMaterial(&newmaterial))
  {
    materials.ReturnElement(*_material);
    *_material = Ice::null32;
    IceLogError("Failed to create material");
    return false;
  }

  // Bind default descriptors (create buffer, assign textures)

  return true;
}

void Ice::SetMaterialData(Ice::Material* _material, Ice::BufferSegment _segment, void* _data)
{
  _segment.buffer = &_material->buffer;
  renderer->PushDataToBuffer(_data, _segment);
}

b8 Ice::ReloadShader(Shader* _shader)
{
  return renderer->ReloadShader(_shader);
}

b8 Ice::ReloadAllShaders()
{
  for (Ice::Shader& s : shaders)
  {
    ICE_ATTEMPT(ReloadShader(&s));
  }

  return true;
}

b8 Ice::RecreateMaterial(Material* _material)
{
  return renderer->RecreateMaterial(_material);
}

b8 Ice::RecreateAllMaterials()
{
  for (Ice::Material& m : materials)
  {
    ICE_ATTEMPT(RecreateMaterial(&m));
  }
  return true;
}

b8 Ice::ReloadMaterial(Material* _material)
{
  for (u32 i = 0; i < _material->shaders.size(); i++)
  {
    ICE_ATTEMPT(ReloadShader(_material->shaders[i]));
  }

  return RecreateMaterial(_material);
}

b8 Ice::ReloadAllMaterials()
{
  ICE_ATTEMPT(ReloadAllShaders());
  return RecreateAllMaterials();
}

b8 Ice::LoadTexture(Ice::Image* _texture, const char* _directory)
{
  _texture->extents = { 0, 0 };

  void* imageData = Ice::LoadImageFile(_directory, &_texture->extents);
  ICE_ASSERT(imageData != nullptr);

  ICE_ATTEMPT(renderer->CreateTexture(_texture, imageData));

  Ice::DestroyImageFile(imageData);
  return true;
}

void Ice::SetTexture(Ice::Material* _material, u32 _inputIndex, const char* _directory)
{
  Ice::Image& newTexture = textures.GetNewElement();
  LoadTexture(&newTexture, _directory);

  renderer->SetMaterialInput(_material, _inputIndex, &newTexture);
}

Ice::Entity Ice::CreateCamera(Ice::CameraSettings _settings /*= {}*/)
{
  Ice::Entity e = Ice::CreateEntity();
  Ice::CameraComponent* cc = e.AddComponent<Ice::CameraComponent>();
  e.AddComponent<Ice::CameraData>();
  Ice::Transform* t = e.AddComponent<Ice::Transform>();

  t->bufferSegment.buffer = &cc->buffer;
  t->bufferSegment.count = 1;
  t->bufferSegment.elementSize = sizeof(mat4);
  t->bufferSegment.startIndex = e.id;
  t->bufferSegment.offset = 0;

  renderer->InitializeCamera(cc, t->bufferSegment, _settings);

  return e;
}

Ice::Entity Ice::CreateRenderedEntity(const char* _meshDir /*= nullptr*/,
                                      u32 _material /*= Ice::null32*/)
{
  Ice::CompactArray<Ice::Transform>& carray = Ice::GetComponentArray<Ice::Transform>();
  Ice::Entity e = Ice::CreateEntity();
  if (e == nullEntity)
    return e;

  Ice::GetComponentArray<Ice::Transform>();
  Ice::Transform* t = e.AddComponent<Ice::Transform>();

  t->bufferSegment.buffer = &transformsBuffer;
  t->bufferSegment.count = 1;
  t->bufferSegment.elementSize = sizeof(mat4);
  t->bufferSegment.startIndex = e.id;
  t->bufferSegment.offset = 0;

  if (Ice::GetComponentArray<Ice::Transform>().GetAllocatedSize() > transformsBuffer.count)
  {
    Ice::CompactArray<Ice::Transform>& transformCompact = Ice::GetComponentArray<Ice::Transform>();
    renderer->ResizeBufferMemory(&transformsBuffer, transformCompact.GetAllocatedSize());

    for (Ice::Entity e : Ice::SceneView<Ice::RenderComponent, Ice::Transform>())
    {
      transformCompact[e].bufferSegment.buffer = &transformsBuffer;
      renderer->UpdateRenderComponent(e.GetComponent<Ice::RenderComponent>(),
                                      transformCompact[e].bufferSegment);
    }
  }

  Ice::RenderComponent* r = e.AddComponent<Ice::RenderComponent>();
  renderer->InitializeRenderComponent(r, t->bufferSegment);

  if (_meshDir != nullptr)
  {
    if (!Ice::GetMesh(_meshDir, &r->mesh))
    {
      return e;
    }

    if (_material != Ice::null32)
    {
      r->material = _material;
    }
  }

  return e;
}

b8 Ice::UpdateTransforms()
{
  Ice::CompactArray<Ice::Transform>& transformCompact = Ice::GetComponentArray<Ice::Transform>();

  if (transformCompact.GetAllocatedSize() > transformsBuffer.count)
  {
    renderer->ResizeBufferMemory(&transformsBuffer, transformCompact.GetAllocatedSize());

    for (Ice::Entity e : Ice::SceneView<Ice::RenderComponent, Ice::Transform>())
    {
      transformCompact[e].bufferSegment.buffer = &transformsBuffer;
      renderer->UpdateRenderComponent(e.GetComponent<Ice::RenderComponent>(),
                                      transformCompact[e].bufferSegment);
    }
  }

  std::vector<Ice::Entity> camEntities;
  for (Ice::Entity e : Ice::SceneView<Ice::Transform, Ice::CameraComponent>())
  {
    camEntities.push_back(e);
  }

  u32 count = 0;
  Ice::Transform* transforms = transformCompact.GetArray(&count);
  Ice::mat4 m(1.0f);
  for (u32 i = 0, j = 0; i < count; i++)
  {
    if (transforms[i].GetDirty()) // TODO : Separate static & dynamic transforms
    {
      if (j < camEntities.size() && i == transformCompact.GetMappedIndex(camEntities[j].id))
      {
        Ice::CameraData* cd = camEntities[j].GetComponent<Ice::CameraData>();
        Ice::CameraComponent* cc = camEntities[j].GetComponent<Ice::CameraComponent>();

        Ice::vec3 v = transforms[i].GetPosition();
        cd->position = Ice::vec4(v.x, v.y, v.z, 1.0f);
        v = transforms[i].ForwardVector();
        cd->forward = Ice::vec4(v.x, v.y, v.z, 1.0f);

        cd->viewProjectionMatrix = transforms[i].GetMatrix().Inverse() * cc->projectionMatrix;

        Ice::BufferSegment segment {};
        segment.buffer = &cc->buffer;
        segment.elementSize = cc->buffer.elementSize;
        segment.count = 1;
        segment.offset = 0;

        renderer->PushDataToBuffer(cd, segment);

        j++;
      }
      else
      {
        m = transforms[i].GetMatrix();
        renderer->PushDataToBuffer(&m, transforms[i].bufferSegment);
      }
    }
  }

  return true;
}
