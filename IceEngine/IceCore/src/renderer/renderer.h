
#ifndef RENDERER_RENDERER_H
#define RENDERER_RENDERER_H 1
// TODO : Define API agnostic calls

#include "defines.h"
#include "renderer/renderer_backend.h"
#include "renderer/shader_program.h"

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

private:
  //void Initialize();
  //void Shutdown();

  void CreateRenderer();
  void CleanupRenderer();
  void RecreateRenderer();

};

#endif // !RENDERER_RENDERER_H
