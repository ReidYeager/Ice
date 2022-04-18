
#include "defines.h"
#include "logger.h"

#include "core/application.h"

#include "core/input.h"
#include "core/ecs.h"
#include "core/scene.h"
#include "platform/platform.h"
#include "rendering/vulkan/vulkan.h"
#include "math/linear.h"

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
Ice::Mesh* meshes;
u32 objectCount = 0;
Ice::Object* objects;

Ice::ECS::ComponentManager<Ice::RenderComponent> renderComponents;
Ice::ECS::ComponentManager<Ice::TransformComponent> transformComponents;
Ice::Buffer transformsBuffer; // Stores the transforms matrix for every object
Ice::ECS::ComponentManager<Ice::CameraComponent> cameraComponents;

//=========================
// Time
//=========================

Ice::IceTime Ice::time;
// Setup time =====
std::chrono::steady_clock::time_point realtimeStart, frameStart, frameEnd;
const float microToSecond = 0.000001f;

void InitTime()
{
  Ice::time.deltaTime = 0.0f;
  Ice::time.totalTime = 0.0f;
  Ice::time.frameCount = 0;

  realtimeStart = std::chrono::steady_clock::now();
  frameStart = frameEnd = realtimeStart;

  Ice::time.deltaTime = 0.0f;
}

void UpdateTime()
{
  frameEnd = std::chrono::steady_clock::now();

  Ice::time.deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - frameStart).count() * microToSecond;
  Ice::time.totalTime = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - realtimeStart).count() * microToSecond;

  Ice::time.frameCount++;

  frameStart = frameEnd;
}

//=========================
// Application
//=========================

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
  if (_settings.renderer.api == Ice::Renderer_Vulkan)
  {
    renderer = new Ice::RendererVulkan();
  }
  else
  {
    IceLogFatal("Selected API not supported");
    return false;
  }

  if (!renderer->Init(_settings.renderer))
  {
    IceLogFatal("Failed to initialize the renderer");
    return false;
  }

  shaders = (Ice::Shader*)Ice::MemoryAllocZero(sizeof(Ice::Shader) * _settings.maxShaderCount);
  materials = (Ice::Material*)Ice::MemoryAllocZero(sizeof(Ice::Material) * _settings.maxMaterialCount);
  materialSettings = (Ice::MaterialSettings*)Ice::MemoryAllocZero(sizeof(Ice::MaterialSettings) * _settings.maxMaterialCount);
  meshes = (Ice::Mesh*)Ice::MemoryAllocZero(sizeof(Ice::Mesh) * _settings.maxMeshCount);
  objects = (Ice::Object*)Ice::MemoryAllocZero(sizeof(Ice::Object) * _settings.maxObjectCount);

  renderComponents.Initialize(_settings.maxObjectCount);
  transformComponents.Initialize(_settings.maxObjectCount);
  renderer->CreateBufferMemory(&transformsBuffer, 64 * _settings.maxObjectCount, Ice::Buffer_Memory_Shader_Read);
  cameraComponents.Initialize(_settings.maxObjectCount);

  // Game =====
  _settings.clientInitFunction();

  UpdateTime();
  IceLogInfo("} Startup %2.3f s", Ice::time.totalTime);

  return true;
}

b8 IceApplicationUpdate()
{
  UpdateTime();

  Ice::FrameInformation frameInfo {};
  frameInfo.components = &renderComponents;
  frameInfo.cameras = &cameraComponents;

  while (isRunning && Ice::platform.Update())
  {
    ICE_ATTEMPT(appSettings.clientUpdateFunction(Ice::time.deltaTime));

    Ice::UpdateTransforms();

    ICE_ATTEMPT(renderer->RenderFrame(&frameInfo));

    Input.Update();
    UpdateTime();
  }

  return true;
}

b8 IceApplicationShutdown()
{
  ICE_ATTEMPT(appSettings.clientShutdownFunction());

  renderComponents.Shutdown();

  cameraComponents.Shutdown();

  renderer->DestroyBufferMemory(&transformsBuffer);
  transformComponents.Shutdown();

  Ice::MemoryFree(objects);

  for (u32 i = 0; i < meshCount; i++)
  {
    renderer->DestroyBufferMemory(&meshes[i].buffer);
  }
  Ice::MemoryFree(meshes);

  for (u32 i = 0; i < materialCount; i++)
  {
    renderer->DestroyMaterial(materials[i]);
    materialSettings[i].input.clear();
  }
  Ice::MemoryFree(materials);
  Ice::MemoryFree(materialSettings);

  for (u32 i = 0; i < shaderCount; i++)
  {
    renderer->DestroyShader(shaders[i]);
  }
  Ice::MemoryFree(shaders);

  renderer->Shutdown();
  delete(renderer);

  Input.Shutdown();
  Ice::CloseWindow();
  Ice::platform.Shutdown();

  return true;
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
  for (u32 i = 0; i < _settings.shaders.size(); i++)
  {
    shaderFound = false;
    Ice::Shader& newShader = _settings.shaders[i];

    // Check for existing shader
    for (u32 j = 0; j < shaderCount; j++)
    {
      Ice::Shader& oldShader = shaders[j];
      if (newShader.fileDirectory.compare(oldShader.fileDirectory) == 0 &&
          newShader.type == oldShader.type)
      {
        shaderFound = true;
        newShader = oldShader;
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

      if (!renderer->CreateShader(&newShader))
      {
        IceLogError("Failed to create a shader\n> '%s'", newShader.fileDirectory.c_str());
        return false;
      }

      shaders[shaderCount] = newShader;
      shaderCount++;
    }
  }

  materialSettings[materialCount] = _settings;

  // Create material =====
  Ice::Material newMaterial;
  newMaterial.settings = &materialSettings[materialCount];

  if (!renderer->CreateMaterial(&newMaterial))
  {
    IceLogError("Failed to create material");
    return false;
  }
  materials[materialCount] = newMaterial;
  materialCount++;

  if (_material != nullptr)
  {
    *_material = &materials[materialCount - 1];
  }

  // Bind default descriptors (create buffer, assign textures)

  return true;
}

b8 CreateMesh(const char* _directory, Ice::Mesh* _mesh)
{
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  // Load mesh =====
  {
    std::string loadWarnings, loadErrors;

    std::string dir(ICE_RESOURCE_MODEL_DIR);
    dir.append(_directory);

    ICE_ATTEMPT_MSG(tinyobj::LoadObj(&attrib,
                                      &shapes,
                                      &materials,
                                      &loadWarnings,
                                      &loadErrors,
                                      dir.c_str()),
                    Ice::Log_Error,
                    "Failed to load mesh '%s'\n> tinyobj warnings: '%s'\n> tinyobj errors: '%s'",
                    _directory,
                    loadWarnings.c_str(),
                    loadErrors.c_str());
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
    Ice::Mesh newMesh {};

    newMesh.indexCount = indices.size();
    ICE_ATTEMPT(renderer->CreateBufferMemory(&newMesh.buffer,
                                             (sizeof(Ice::Vertex) * vertices.size() +
                                               (sizeof(u32) * indices.size())),
                                             Ice::Buffer_Memory_Vertex | Ice::Buffer_Memory_Index));

    newMesh.vertexBuffer.ivkBuffer = newMesh.buffer.ivkBuffer;
    newMesh.vertexBuffer.offset = 0;
    newMesh.vertexBuffer.size = sizeof(Ice::Vertex) * vertices.size();
    if (!renderer->PushDataToBuffer(vertices.data(), &newMesh.buffer, newMesh.vertexBuffer))
    {
      IceLogError("Failed to push vertex data to its buffer");
      renderer->DestroyBufferMemory(&newMesh.buffer);
      return false;
    }

    newMesh.indexBuffer.ivkBuffer = newMesh.buffer.ivkBuffer;
    newMesh.indexBuffer.offset = newMesh.vertexBuffer.size;
    newMesh.indexBuffer.size = sizeof(u32) * indices.size();
    if (!renderer->PushDataToBuffer(indices.data(), &newMesh.buffer, newMesh.indexBuffer))
    {
      IceLogError("Failed to push index data to its buffer");
      renderer->DestroyBufferMemory(&newMesh.buffer);
      return false;
    }

    meshes[meshCount] = newMesh;
    *_mesh = meshes[meshCount];
    meshCount++;
  }

  return true;
}

Ice::Object& Ice::CreateObject()
{
  Ice::Object entity(Ice::ECS::CreateEntity());

  Ice::TransformComponent* tc = transformComponents.Create(entity.GetId());
  u32 index = transformComponents.GetIndex(entity.GetId());

  tc->bufferSegment.offset = index * 64;
  tc->bufferSegment.size = 64;
  tc->bufferSegment.apiData0 = transformsBuffer.apiData0;
  //renderer->CreateBufferMemory(&tc->buffer, 64, Ice::Buffer_Memory_Shader_Read);
  renderer->PushDataToBuffer((void*)&mat4Identity, &transformsBuffer, tc->bufferSegment);

  entity.transform = &tc->transform;

  objects[objectCount] = entity;
  objectCount++;

  return objects[objectCount - 1];
}

void Ice::AttatchRenderComponent(Ice::Object* _object,
                                 const char* _meshDir,
                                 Ice::Material* _material)
{
  Ice::RenderComponent* rc = renderComponents.Create(_object->GetId());
  Ice::BufferSegment* tbuffer = &transformComponents.GetComponent(_object->GetId())->bufferSegment;

  renderer->InitializeRenderComponent(rc, tbuffer);
  CreateMesh(_meshDir, &rc->mesh);
  rc->material = *_material;
}

void Ice::AttatchCameraComponent(Ice::Object* _object, Ice::CameraSettings _settings)
{
  Ice::CameraComponent* cam = cameraComponents.Create(_object->GetId());
  renderer->InitializeCamera(cam,
                             transformComponents.GetComponent(_object->GetId())->bufferSegment,
                             _settings);

  IceLogDebug("Camera created");
}

void Ice::UpdateTransforms()
{
  for (u32 i = 0; i < transformComponents.GetCount(); i++)
  {
    vec3 s = transformComponents[i].transform.scale;
    vec3 p = transformComponents[i].transform.position;
    vec3 r = transformComponents[i].transform.rotation * 0.008726646f; // (Degrees to Radians)/2

    Ice::CameraComponent* cam = cameraComponents.GetComponent(transformComponents.GetEntity(i));
    if (cam == nullptr)
    {
      // Calculate quaternion =====
      // Rotate round each local-space axis
      vec4 q1 = {0, sin(r.y), 0, cos(r.y)}; // Yaw
      vec4 q2 = {sin(r.x), 0, 0, cos(r.x)}; // Pitch
      vec4 q3 = {0, 0, sin(r.z), cos(r.z)}; // Roll

      // Combine into a quaternion rotated by q1, then q2, then q3
      vec4 q12 = {
        q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y, // X
        q1.w * q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z, // Y
        q1.w * q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x, // Z
        q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z  // W
      };
      vec4 q = {
        q12.w * q3.x + q12.x * q3.w + q12.y * q3.z - q12.z * q3.y, // X
        q12.w * q3.y + q12.y * q3.w + q12.z * q3.x - q12.x * q3.z, // Y
        q12.w * q3.z + q12.z * q3.w + q12.x * q3.y - q12.y * q3.x, // Z
        q12.w * q3.w - q12.x * q3.x - q12.y * q3.y - q12.z * q3.z  // W
      };

      // Combine scale, rotation, position =====
      mat4 combined = {
        s.x*(2*(q.w*q.w+q.x*q.x)-1), 2*s.y*(q.x*q.y-q.w*q.z)    , 2*s.z*(q.w*q.y+q.x*q.z)    , p.x,
        2*s.x*(q.w*q.z+q.x*q.y)    , s.y*(2*(q.w*q.w+q.y*q.y)-1), 2*s.z*(q.y*q.z-q.w*q.x)    , p.y,
        2*s.x*(q.x*q.z-q.w*q.y)    , 2*s.y*(q.w*q.x+q.y*q.z)    , s.z*(2*(q.w*q.w+q.z*q.z)-1), p.z,
        0                          , 0                          , 0                          , 1
      };
      combined = combined.Transpose();

      // Update buffer =====
      renderer->PushDataToBuffer((void*)&combined,
                                 &transformsBuffer,
                                 transformComponents[i].bufferSegment);
    }
    else
    {
      s = { 1.0f / s.x, 1.0f / s.y, 1.0f / s.z };
      p *= -1;
      r *= -1;

      // Calculate quaternion =====
      // Rotate round each local-space axis
      vec4 q3 = {0, sin(r.y), 0, cos(r.y)}; // Yaw
      vec4 q2 = {sin(r.x), 0, 0, cos(r.x)}; // Pitch
      vec4 q1 = {0, 0, sin(r.z), cos(r.z)}; // Roll

      // Combine into a quaternion rotated by q1, then q2, then q3
      vec4 q12 = {
        q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y, // X
        q1.w * q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z, // Y
        q1.w * q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x, // Z
        q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z  // W
      };
      vec4 q = {
        q12.w * q3.x + q12.x * q3.w + q12.y * q3.z - q12.z * q3.y, // X
        q12.w * q3.y + q12.y * q3.w + q12.z * q3.x - q12.x * q3.z, // Y
        q12.w * q3.z + q12.z * q3.w + q12.x * q3.y - q12.y * q3.x, // Z
        q12.w * q3.w - q12.x * q3.x - q12.y * q3.y - q12.z * q3.z  // W
      };

      // Combine scale, rotation, position =====
      mat4 combined = {
        s.x*(2*(q.w*q.w+q.x*q.x)-1), 2*s.y*(q.x*q.y-q.w*q.z)    , 2*s.z*(q.w*q.y+q.x*q.z)    , s.x*p.x*(2*(q.w*q.w+q.x*q.x)-1)+2*s.x*p.y*(q.x*q.y-q.w*q.z)+2*s.x*p.z*(q.w*q.y+q.x*q.z),
        2*s.x*(q.w*q.z+q.x*q.y)    , s.y*(2*(q.w*q.w+q.y*q.y)-1), 2*s.z*(q.y*q.z-q.w*q.x)    , 2*s.y*p.x*(q.w*q.z+q.x*q.y)+s.y*p.y*(2*(q.w*q.w+q.y*q.y)-1)+2*s.y*p.z*(q.y*q.z-q.w*q.x),
        2*s.x*(q.x*q.z-q.w*q.y)    , 2*s.y*(q.w*q.x+q.y*q.z)    , s.z*(2*(q.w*q.w+q.z*q.z)-1), 2*s.z*p.x*(q.x*q.z-q.w*q.y)+2*s.z*p.y*(q.w*q.x+q.y*q.z)+s.z*p.z*(2*(q.w*q.w+q.z*q.z)-1),
        0                          , 0                          , 0                          , 1
      };
      combined = combined.Transpose();

      // Update buffer =====
      // Fastest way to convert to a column-major glm matrix (f32[16]); avoids copying data
      glm::mat4* transform = (glm::mat4*)&combined;
      mat4 cm = cam->projectionMatrix.Transpose();
      glm::mat4* projection = (glm::mat4*)&cm;

      glm::mat4 projView = *projection * *transform;

      renderer->PushDataToBuffer((void*)&projView,
                                 &transformsBuffer,
                                 transformComponents[i].bufferSegment);
    }
  }

}
