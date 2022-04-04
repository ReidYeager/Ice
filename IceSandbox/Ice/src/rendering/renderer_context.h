
#ifndef ICE_RENDERING_RENDERER_CONTEXT_H_
#define ICE_RENDERING_RENDERER_CONTEXT_H_

#include "defines.h"

#include "core/ecs.h"

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
    u64 size;

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
    u64 offset; // Offset of this segment's start within the original buffer
    u64 size;   // Size of this segment

    // The segment's parent buffer
    union {
      void* apiData0;
      VkBuffer ivkBuffer;
    };
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

  #define buf(name, index) Shader_Buffer_##name = (1 << index)
  enum ShaderBufferComponentBits : u64
  {
    buf(Custom_0, 0),
    buf(Custom_1, 1),
    buf(Custom_2, 2),
    buf(Custom_3, 3),

    Shader_Buffer_Count = 4
  };
  typedef IceFlagExtended ShaderBufferComponentFlags;
  #undef buf

  #define buf(name, index) #name
  const char* const ShaderBufferComponentStrings[Shader_Buffer_Count] = {
    buf(Custom_0, 0),
    buf(Custom_1, 1),
    buf(Custom_2, 2),
    buf(Custom_3, 3)
  };
  #undef buf

  // Material proper =====

  struct ShaderInputElement
  {
    ShaderInputTypes type = Ice::Shader_Input_Count;
    u32 inputIndex; // Opengl matID, Vulkan binding index, ...
    void* assignedData; // 
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
    ShaderBufferComponentFlags bufferComponents;

    // I'm not 100% sure how I feel about this. It makes the code more readable,
    //  but it feels dirty letting API info creep beyond its usage files.
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
    Ice::ShaderBufferComponentFlags bufferComponents;
  };

  struct Material
  {
    Ice::MaterialSettings* settings;

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

  struct RenderComponent
  {
    Ice::Material material;
    Ice::Mesh mesh;

    union {
      void* apiData0;
      VkDescriptorSet ivkDescriptorSet; // Per-object shader material input data
    };
  };

  // The contents of this struct are currently in flux.
  // A permanent solution will be settled on eventually.
  struct FrameInformation
  {
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
