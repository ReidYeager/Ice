
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

  enum GpuMemoryUsage
  {
    Gpu_Memory_Shader_Read = 0x01,
    Gpu_Memory_Vertex = 0x02,
    Gpu_Memory_Index = 0x04
  };

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
  // Renderer
  //=========================

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
