
#ifndef ICE_RENDERER_RENDERER_H_
#define ICE_RENDERER_RENDERER_H_
// TODO : Define API agnostic calls

#include "defines.h"
#include "renderer/renderer_backend.h"
#include "renderer/shader_program.h"
#include "renderer/mesh.h"

#include <glm/glm.hpp>
#include <vector>

class IceRenderer
{
public:
  enum IceRenderingAPI
  {
    Vulkan // Ivk*
    //OpenGL, // Igl*
    //DirectX // Idx*
  };

//=================================================
// Variables
//=================================================
private:
  RendererBackend* backend = nullptr;

  std::vector<iceShaderProgram_t> shaderPrograms;
  std::vector<iceShader_t> shaders;

  //std::vector<VkBuffer> buffers;
  //std::vector<VkDeviceMemory> bufferMemories;

//=================================================
// Functions
//=================================================
public:
  void Initialize(IceRenderingAPI _api);
  void Shutdown();

  void RecordCommandBuffers();
  void RenderFrame(IceRenderPacket* _packet);

  // Returns the index of the shader program
  // Creates a new shader if none match the given description
  u32 GetShaderProgram(IceRenderContext* rContext, const char* _name, IceShaderStageFlags _stages,
                       std::vector<const char*> _texStrings,
                       IcePipelineSettingFlags _settings = Ice_Pipeline_Default);
  // Returns the index of the shader
  // Creates a new shader if none match the given description
  //u32 GetShader(const char* _name, IceShaderStageFlags _stage);

  //u32 CreateBuffer(const void* _data, VkDeviceSize _size, VkBufferUsageFlags _usage);
  //void DestroyBuffer(u32 _index);

  mesh_t CreateMesh(const char* _model);

  void Resize(u32 _width = 0, u32 _height = 0);

  IceRenderContext* GetContext() { return backend->GetContext(); }

};

extern IceRenderer Renderer;

#endif // !RENDERER_RENDERER_H
