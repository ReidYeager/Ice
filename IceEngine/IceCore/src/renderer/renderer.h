
#ifndef RENDERER_RENDERER_H
#define RENDERER_RENDERER_H 1
// TODO : Define API agnostic calls

#include "defines.h"
#include "renderer/renderer_backend.h"
#include "renderer/shader_program.h"
#include "renderer/mesh.h"

#include <vector>

class Renderer
{
//=================================================
// Variables
//=================================================
private:
  RendererBackend* backend = nullptr;

  std::vector<iceShaderProgram_t> shaderPrograms;
  std::vector<iceShader_t> shaders;

  std::vector<VkBuffer> buffers;
  std::vector<VkDeviceMemory> bufferMemories;

//=================================================
// Functions
//=================================================
public:
  Renderer();
  ~Renderer();

  void RenderFrame();

  // Returns the index of the shader program
  // Creates a new shader if none match the given description
  u32 GetShaderProgram(const char* _name, IceShaderStageFlags _stages);
  // Returns the index of the shader
  // Creates a new shader if none match the given description
  u32 GetShader(const char* _name, IceShaderStageFlags _stage);

  u32 CreateBuffer(const void* _data, VkDeviceSize _size, VkBufferUsageFlags _usage);
  void DestroyBuffer(u32 _index);

  mesh_t CreateMesh(const char* _model);

private:
  //void Initialize();
  //void Shutdown();

  void CreateRenderer();
  void CleanupRenderer();
  void RecreateRenderer();

};

#endif // !RENDERER_RENDERER_H
