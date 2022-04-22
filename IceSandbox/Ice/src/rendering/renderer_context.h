
#ifndef ICE_RENDERING_RENDERER_CONTEXT_H_
#define ICE_RENDERING_RENDERER_CONTEXT_H_

#include "defines.h"

#include "core/ecs.h"
#include "math/matrix.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <string>

#define ALIGN_FOR_SHADER __declspec(align(16)) // Shaders are 16-byte aligned

namespace Ice {

  //=========================
  // Buffer
  //=========================

  enum BufferMemoryUsageBits
  {
    Buffer_Memory_Shader_Read = 0x01,
    Buffer_Memory_Vertex = 0x02,
    Buffer_Memory_Index = 0x04
  };
  typedef IceFlag BufferMemoryUsageFlags;

  struct Buffer
  {
    u64 elementSize = 0; // Bytes
    u64 padElementSize = 0; // Bytes, a multiple of an alignment value (API dependent)
    u32 count = 1; // Number of elements
    BufferMemoryUsageFlags usage;

    union {
      void* apiData0;
      VkBuffer ivkBuffer;
    };

    union {
      void* apiData1;
      VkDeviceMemory ivkMemory;
    };
  };

  struct BufferSegment
  {
    u64 elementSize; // Number of bytes affected in each covered element
    u32 startIndex; // Index of the first covered element
    u64 offset; // Offset into the first covered element
    u32 count; // Number of covered elements

    // The segment's parent buffer
    Ice::Buffer* buffer;
  };

  //=========================
  // Material
  //=========================

  #define si(name) Shader_Input_##name
  enum ShaderInputTypes
  {
    si(Buffer),
    si(Image),

    Shader_Input_Count,
  };
  #undef si

  #define si(name) #name
  const char* const ShaderInputTypeStrings[Shader_Input_Count] = {
    si(Buffer),
    si(Image)
  };
  #undef si

  // Material proper =====

  struct ShaderInputElement
  {
    ShaderInputTypes type = Ice::Shader_Input_Count;
    u32 inputIndex; // Opengl matID, Vulkan binding index, ...

    union {
      void* apiData0;
      Ice::BufferSegment bufferSegment;
      // Image
    };
  };

  enum ShaderTypes
  {
    Shader_Unknown = 0,
    Shader_Vertex = 1,
    Shader_Fragment = 2,
    Shader_Compute = 4,
    // ...
  };

  struct Shader
  {
    ShaderTypes type;
    std::string fileDirectory;
    std::vector<ShaderInputElement> input;

    union {
      void* apiData0;
      VkShaderModule ivkShaderModule;
    };
  };

  struct MaterialSettings
  {
    std::vector<Ice::Shader> shaders;
    u32 subpassIndex = 0;

    std::vector<ShaderInputElement> input;
  };

  struct Material
  {
    Ice::MaterialSettings* settings;
    Ice::Buffer buffer;

    union {
      void* apiData0;
      VkPipelineLayout ivkPipelineLayout;
    };

    union {
      void* apiData1;
      VkPipeline ivkPipeline;
    };

    union {
      void* apiData2;
      VkDescriptorSetLayout ivkDescriptorSetLayout;
    };

    union {
      void* apiData3;
      VkDescriptorSet ivkDescriptorSet;
    };
  };

  //=========================
  // Mesh
  //=========================

  struct Vertex
  {
    vec3 position;
    vec2 uv;
    vec3 normal;
  };

  struct Mesh
  {
    //std::vector<Ice::Vertex> vertices;
    //std::vector<u32> indices;
    u32 indexCount;
    Ice::Buffer buffer;
    Ice::BufferSegment vertexBuffer;
    Ice::BufferSegment indexBuffer;
  };

  //=========================
  // Renderer
  //=========================

  struct CameraSettings
  {
    b8 isProjection = true;

    f32 horizontal; // If projection: the horizontal FOV; else: the world-space width
    f32 ratio; // The ratio of screen width:height

    f32 nearClip = 0.1f;
    f32 farClip = 100.0f;
  };

  // TODO : Add camera render target
  struct CameraComponent
  {
    mat4 projectionMatrix;

    union {
      void* apiData0;
      VkDescriptorSet ivkDescriptorSet; // ProjectionViewMatrix buffer
    };
  };

  struct RenderComponent
  {
    // TODO : Rework the renderComponent to not chase pointers
    Ice::Material* material;
    Ice::Mesh* mesh;

    union {
      void* apiData0;
      VkDescriptorSet ivkDescriptorSet; // Per-object input data
    };
  };

  // The contents of this struct are currently in flux.
  // A permanent solution will be settled on eventually.
  struct FrameInformation
  {
    Ice::ECS::ComponentManager<Ice::CameraComponent>* cameras;
    Ice::ECS::ComponentManager<Ice::RenderComponent>* components;
  };

  enum RenderingApi
  {
    Renderer_Unknown,
    Renderer_Vulkan,
    Renderer_OpenGL,
    Renderer_DirectX
  };

  struct RendererSettings
  {
    //Ice::Renderer* existingRenderer = nullptr; // Used to setup resources for a new window
    Ice::RenderingApi api;
  };

}

#endif // !define ICE_RENDERING_RENDERER_CONTEXT_H_
