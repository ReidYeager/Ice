
#ifndef ICE_RENDERING_RENDERER_CONTEXT_H_
#define ICE_RENDERING_RENDERER_CONTEXT_H_

#include "defines.h"

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

  enum ShaderInputTypes
  {
    Shader_Input_Buffer,
    Shader_Input_Image2D,

    Shader_Input_Count,
  };

  const char* const ShaderInputTypeStrings[Shader_Input_Count] = {
    "buffer",
    "image2d"
  };

  enum ShaderBufferComponents
  {
    Shader_Buffer_Custom_0,
    Shader_Buffer_Custom_1,
    Shader_Buffer_Custom_2,
    Shader_Buffer_Custom_3,

    Shader_Buffer_Count
  };

  const char* const ShaderBufferComponentStrings[Shader_Buffer_Count] = {
    "custom_0",
    "custom_1",
    "custom_2",
    "custom_3"
  };

  // Material proper =====

  struct ShaderInputElement
  {
    ShaderInputTypes type = Ice::Shader_Input_Count;
    u32 inputIndex; // Opengl matID, Vulkan binding index, ...
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
    u32 bufferSize;

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
  };

  struct Material
  {
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

    // Required for hash mapping
    // Compares the attributes of other against itself
    bool operator==(const Ice::Vertex& other) const
    {
      return position == other.position && normal == other.normal && uv == other.uv;
    }

    static VkVertexInputBindingDescription GetBindingDescription()
    {
      VkVertexInputBindingDescription desc = {};
      desc.stride = sizeof(Ice::Vertex);
      desc.binding = 0;
      desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      return desc;
    }

    static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions()
    {
      std::vector<VkVertexInputAttributeDescription> attribs(3);
      // Position
      attribs[0].binding = 0;
      attribs[0].location = 0;
      attribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribs[0].offset = offsetof(Ice::Vertex, position);
      // UV
      attribs[2].binding = 0;
      attribs[2].location = 1;
      attribs[2].format = VK_FORMAT_R32G32_SFLOAT;
      attribs[2].offset = offsetof(Ice::Vertex, uv);
      // normal
      attribs[1].binding = 0;
      attribs[1].location = 2;
      attribs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribs[1].offset = offsetof(Ice::Vertex, normal);

      return attribs;
    }
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
    Ice::Material* materials;
    u32 materialCount;
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
