
#include "defines.h"

#include "core/application.h"

#include "core/input.h"
#include "core/ecs/ecs.h"
#include "platform/platform.h"
#include "rendering/vulkan/vulkan.h"
#include "math/linear.h"
#include "math/transform.h"
#include "core/ecs/entity.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>

Ice::ApplicationSettings appSettings;
Ice::Renderer* renderer;
b8 isRunning;

// Rendering =====
u32 shaderCount = 0;
Ice::Shader* shaders;

u32 materialCount = 0;
Ice::Material* materials;
Ice::MaterialSettings* materialSettings;

u32 meshCount = 0;
Ice::MeshInformation* meshes;
u32 maxMeshCount = 0;

Ice::Image* textures;
u32 textureCount = 0;

Ice::FrameInformation frameInfo{};
std::vector<Ice::Scene*> scenes;
Ice::Scene* activeScene = nullptr;

Ice::Buffer transformsBuffer;

Ice::Scene* mainScene;

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
  Input.Initialize();

  // Rendering =====
  if (_settings.rendererCore.api == Ice::RenderApiVulkan)
  {
    renderer = new Ice::RendererVulkan();
  }
  else
  {
    IceLogFatal("Selected API not supported");
    return false;
  }

  if (!renderer->Init(_settings.rendererCore, _settings.window.title, _settings.version))
  {
    IceLogFatal("Failed to initialize the renderer");
    return false;
  }

  shaders = (Ice::Shader*)Ice::MemoryAllocZero(sizeof(Ice::Shader) * _settings.maxShaderCount);
  materials = (Ice::Material*)Ice::MemoryAllocZero(sizeof(Ice::Material) * _settings.maxMaterialCount);
  materialSettings = (Ice::MaterialSettings*)Ice::MemoryAllocZero(sizeof(Ice::MaterialSettings) * _settings.maxMaterialCount);
  meshes = (Ice::MeshInformation*)Ice::MemoryAllocZero(sizeof(Ice::MeshInformation) * _settings.maxMeshCount);
  maxMeshCount = _settings.maxMeshCount;
  textures = (Ice::Image*)Ice::MemoryAllocZero(sizeof(Ice::Image) * _settings.maxTextureCount);

  ICE_ATTEMPT(renderer->CreateBufferMemory(&transformsBuffer,
                                           sizeof(Ice::mat4),
                                           _settings.maxObjectCount,
                                           Ice::Buffer_Memory_Shader_Read));

  // Initialize transforms =====
  Ice::Transform* transforms = Ice::GetComponentArray<Ice::Transform>().GetArray();
  u32 transformCount = Ice::GetComponentArray<Ice::Transform>().GetAllocatedSize();
  for (u32 i = 0; i < transformCount; i++)
  {
    transforms[i] = Ice::Transform();
  }

  // Game =====
  _settings.GameInit();

  UpdateTime();
  IceLogInfo("} Startup %2.3f s", Ice::time.timeSinceStart);

  return true;
}

b8 IceApplicationUpdate()
{
  UpdateTime();

  Ice::FrameInformation frameInfo;
  frameInfo.cameras = &Ice::GetComponentArray<Ice::CameraComponent>();
  frameInfo.renderables = &Ice::GetComponentArray<Ice::RenderComponent>();

  while (isRunning && Ice::platform.Update())
  {
    ICE_ATTEMPT(appSettings.GameUpdate(Ice::time.deltaTime));

    Ice::UpdateTransforms();

    ICE_ATTEMPT(renderer->RenderFrame(&frameInfo));

    Input.Update();
    UpdateTime();
  }

  return true;
}

b8 IceApplicationShutdown()
{
  ICE_ATTEMPT(appSettings.GameShutdown());

  for (u32 i = 0; i < scenes.size(); i++)
  {
    delete(scenes[i]);
  }
  scenes.clear();

  for (u32 i = 0; i < textureCount; i++)
  {
    renderer->DestroyImage(&textures[i]);
  }
  Ice::MemoryFree(textures);

  for (u32 i = 0; i < meshCount; i++)
  {
    renderer->DestroyBufferMemory(&meshes[i].mesh.buffer);
  }
  Ice::MemoryFree(meshes);

  for (u32 i = 0; i < materialCount; i++)
  {
    renderer->DestroyMaterial(materials[i]);
    //materials[i].input.clear();
  }
  Ice::MemoryFree(materials);
  Ice::MemoryFree(materialSettings);

  for (u32 i = 0; i < shaderCount; i++)
  {
    renderer->DestroyShader(shaders[i]);
  }
  Ice::MemoryFree(shaders);

  renderer->DestroyBufferMemory(&transformsBuffer);

  renderer->Shutdown();
  delete(renderer);

  Input.Shutdown();
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
  template<> struct hash<HashedVertex> {
    size_t operator()(HashedVertex const& vertex) const {
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
    HashedVertex vert {};
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

          Ice::Vertex properVert {};
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

b8 Ice::GetMesh(const char* _directory, Ice::Mesh** _mesh)
{
  for (u32 i = 0; i < meshCount; i++)
  {
    if (meshes[i].fileName.compare(_directory) == 0)
    {
      *_mesh = &meshes[i].mesh;
      return true;
    }
  }

  if (meshCount >= maxMeshCount)
  {
    return false;
  }

  ICE_ATTEMPT(CreateMesh(_directory, &meshes[meshCount].mesh))
  meshes[meshCount].fileName = _directory;
  *_mesh = &meshes[meshCount].mesh;
  meshCount++;

  return true;
}

b8 Ice::CreateMaterial(Ice::MaterialSettings _settings, Ice::Material** _material /*= nullptr*/)
{
  // Don't search for existing materials to allow one material setup with multiple buffer values
  // TODO : Create buffers for multiple instances of a material (instead of creating multiple materials)

  if (materialCount == appSettings.maxMaterialCount)
  {
    IceLogError("Maximum material count (%u) reached", appSettings.maxMaterialCount);
    return false;
  }

  // Get shaders' info =====
  b8 shaderFound = false;
  std::vector<Ice::Shader*> shaderPointers;

  for (u32 i = 0; i < _settings.shaderSettings.size(); i++)
  {
    shaderFound = false;

    // Check for existing shader
    for (u32 j = 0; j < shaderCount; j++)
    {
      Ice::Shader& oldShader = shaders[j];
      if (_settings.shaderSettings[i].fileDirectory.compare(oldShader.settings.fileDirectory) == 0 &&
          _settings.shaderSettings[i].type == oldShader.settings.type)
      {
        shaderFound = true;
        shaderPointers.push_back(&shaders[j]);
        break;
      }
    }

    // Create new shader
    if (!shaderFound)
    {
      if (shaderCount == appSettings.maxShaderCount)
      {
        IceLogError("Maximum shader count (%u) reached", appSettings.maxShaderCount);
        return false;
      }

      if (!renderer->CreateShader(_settings.shaderSettings[i], &shaders[shaderCount]))
      {
        IceLogError("Failed to create a shader\n> '%s'", shaders[shaderCount].settings.fileDirectory.c_str());
        return false;
      }

      shaderPointers.push_back(&shaders[shaderCount]);
      shaderCount++;
    }
  }

  materialSettings[materialCount] = _settings;

  // Create material =====
  materials[materialCount].subpassIndex = _settings.subpassIndex;
  materials[materialCount].shaders = shaderPointers;
  if (!renderer->CreateMaterial(&materials[materialCount]))
  {
    IceLogError("Failed to create material");
    return false;
  }
  materialCount++;

  if (_material != nullptr)
  {
    *_material = &materials[materialCount - 1];
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
  for (u32 i = 0; i < shaderCount; i++)
  {
    ICE_ATTEMPT(ReloadShader(&shaders[i]));
  }

  return true;
}

b8 Ice::RecreateMaterial(Material* _material)
{
  return renderer->RecreateMaterial(_material);
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
  for (u32 i = 0; i < materialCount; i++)
  {
    ICE_ATTEMPT(RecreateMaterial(&materials[i]));
  }
  return true;
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
  Ice::Image& newTexture = textures[textureCount];
  LoadTexture(&newTexture, _directory);

  renderer->SetMaterialInput(_material, _inputIndex, &newTexture);

  textureCount++;
}

/*
Ice::Scene* Ice::CreateScene(u32 _maxObjectCount / *= 100* /, u32 _maxComponentTypeCount / *= 10* /)
{
  scenes.push_back(new Ice::Scene(_maxObjectCount, _maxComponentTypeCount));
  return scenes.back();
}

void Ice::DestroyScene(Ice::Scene* _scene)
{
  for (u32 i = 0; i < scenes.size(); i++)
  {
    if (_scene == scenes[i])
    {
      delete(scenes[i]);

      if (i < scenes.size() - 1)
      {
        scenes[i] = scenes.back();
        scenes.pop_back();
      }
    }
  }

  IceLogError("Scene not found in the application");
}

void Ice::SetActiveScene(Ice::Scene* _scene)
{
  activeScene = _scene;
}

Ice::Scene* Ice::GetActiveScene()
{
  return activeScene;
}

void Ice::AddSceneToRender(Ice::Scene* _scene)
{
  u32 count = 0;

  frameInfo.sceneObjects.push_back(_scene->GetComponentManager<Ice::RenderComponent>());
  frameInfo.sceneCameras.push_back(_scene->GetComponentManager<Ice::CameraComponent>());
}
*/

Ice::Entity Ice::CreateCamera(Ice::Scene* _scene, Ice::CameraSettings _settings /*= {}*/)
{
  Ice::Entity e = _scene->CreateEntity();
  Ice::CameraComponent* cc = _scene->AddComponent<Ice::CameraComponent>(e);
  Ice::Transform* t = _scene->AddComponent<Ice::Transform>(e);

  t->bufferSegment.buffer = &transformsBuffer;
  t->bufferSegment.count = 1;
  t->bufferSegment.elementSize = sizeof(mat4);
  t->bufferSegment.startIndex = e.id;
  t->bufferSegment.offset = 0;

  renderer->InitializeCamera(cc, t->bufferSegment, _settings);

  return e;
}

Ice::Entity Ice::CreateRenderedEntity(Ice::Scene* _scene,
                                      const char* _meshDir /*= nullptr*/,
                                      Ice::Material* _material /*= nullptr*/)
{
  Ice::Entity e = _scene->CreateEntity();
  if (e == nullEntity)
    return e;

  Ice::Transform* t = _scene->AddComponent<Ice::Transform>(e);

  t->bufferSegment.buffer = &transformsBuffer;
  t->bufferSegment.count = 1;
  t->bufferSegment.elementSize = sizeof(mat4);
  t->bufferSegment.startIndex = e.id;
  t->bufferSegment.offset = 0;

  Ice::RenderComponent* r = _scene->AddComponent<Ice::RenderComponent>(e);

  renderer->InitializeRenderComponent(r, &t->bufferSegment);

  if (_meshDir != nullptr)
  {
    if (!Ice::GetMesh(_meshDir, &r->mesh))
    {
      return e;
    }

    if (_material != nullptr)
    {
      r->material = _material;
    }
  }

  return e;
}

void Ice::UpdateTransforms()
{
  Ice::CompactArray<Ice::Transform>& transformCompact = Ice::GetComponentArray<Ice::Transform>();

  Ice::SceneView<Ice::Transform, Ice::CameraComponent> camsView(*mainScene);
  std::vector<Ice::Entity> camEntities;
  for (Ice::Entity e : camsView)
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
      if (j < camEntities.size() && i == transformCompact.indexMap[camEntities[j].id])
      {
        m = transforms[i].GetMatrix().Inverse() * mainScene->GetComponent<Ice::CameraComponent>(camEntities[j])->projectionMatrix;
        j++;
      }
      else
      {
        m = transforms[i].GetMatrix();
      }
      renderer->PushDataToBuffer(&m, transforms[i].bufferSegment);
    }
  }
}

void Ice::TMPSetMainScene(Ice::Scene* _scene)
{
  mainScene = _scene;
}
