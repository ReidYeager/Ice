
#ifndef ICE_RENDERING_RENDERER_CONTEXT_H_
#define ICE_RENDERING_RENDERER_CONTEXT_H_

#include "defines.h"
#include "rendering/vulkan/vulkan_defines.h"

#include "core/ecs/ecs.h"
#include "math/matrix.hpp"
#include "platform/compact_array.h"

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
  Buffer_Memory_Index = 0x04,
  Buffer_Memory_Transfer_Src = 0x08,
  Buffer_Memory_Transfer_Dst = 0x10
};
typedef Ice::Flag BufferMemoryUsageFlags;

struct Buffer
{
  u64 elementSize = 0; // Bytes
  u64 padElementSize = 0; // Bytes, a multiple of an alignment value (API dependent)
  u32 count = 1; // Number of elements
  BufferMemoryUsageFlags usage;

  union
  {
    void* apiData0;
    Ice::IvkBuffer vulkan;
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
// Image
//=========================

struct Image
{
  vec2U extents;

  union
  {
    void* apiData0;
    Ice::IvkImage vulkan;
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

// Material proper =====

struct ShaderInputElement
{
  ShaderInputTypes type = Ice::Shader_Input_Count;
  u32 inputIndex; // Opengl matID, Vulkan binding index, ...

  union
  {
    void* apiData0;
    Ice::BufferSegment bufferSegment;
    Ice::Image* image;
  };
};

enum ShaderTypes
{
  Shader_Unknown = 0x00,
  Shader_Vertex = 0x01,
  Shader_Fragment = 0x02,
  Shader_Compute = 0x04,
  Shader_Geometry = 0x08
  // ...
};

struct ShaderSettings
{
  ShaderTypes type;
  std::string fileDirectory;
};

struct Shader
{
  ShaderSettings settings;
  std::vector<ShaderInputElement> input;

  union
  {
    void* apiData0;
    Ice::IvkShader vulkan;
  };
};

struct MaterialSettings
{
  std::vector<Ice::ShaderSettings> shaderSettings;
  u32 subpassIndex = 0;
};

struct Material
{
  std::vector<Ice::Shader*> shaders;
  std::vector<ShaderInputElement> input;
  Ice::Buffer buffer;
  u32 subpassIndex;

  union
  {
    void* apiData0;
    Ice::IvkMaterial vulkan;
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
  u32 indexCount;
  Ice::Buffer buffer;
  Ice::BufferSegment vertexBuffer;
  Ice::BufferSegment indexBuffer;
};

struct MeshInformation
{
  std::string fileName;
  Ice::Mesh mesh;
};

//=========================
// Renderer
//=========================

struct CameraSettings
{
  b8 isPerspective = true;

  union
  {
    f32 height; // World-space height of the rendered rectangle
    f32 verticalFov; // In degrees, For perspective view
  };
  f32 ratio; // The ratio of screen dimensions (width / height)

  f32 nearClip = 0.1f;
  f32 farClip = 100.0f;
};

struct CameraComponent
{
  mat4 projectionMatrix;

  union
  {
    void* apiData0;
    Ice::IvkObjectData vulkan;
  };
};

struct RenderComponent
{
  // TODO : Rework the renderComponent to not chase pointers
  Ice::Material* material;
  Ice::Mesh* mesh;

  union
  {
    void* apiData0;
    Ice::IvkObjectData vulkan;
  };
};

// The contents of this struct are currently in flux.
// A permanent solution will be settled on eventually.
struct FrameInformation
{
  Ice::CompactArray<Ice::CameraComponent>* cameras;
  Ice::CompactArray<Ice::RenderComponent>* renderables;
};

enum RenderingApi
{
  RenderApiUnknown = 0,
  RenderApiVulkan,
  RenderApiOpenGL,
  RenderApiDirectX
};

// Settings that apply to all windows uniformly
struct RendererSettingsCore
{
  Ice::RenderingApi api;
};

} // namespace Ice

#endif // !define ICE_RENDERING_RENDERER_CONTEXT_H_
