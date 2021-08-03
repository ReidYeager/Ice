
#ifndef RENDERER_RENDERER_BACKEND_H
#define RENDERER_RENDERER_BACKEND_H 1

#include "defines.h"
#include "renderer/backend_context.h"
#include "renderer/shader_program.h"
#include "renderer/mesh.h"
#include "renderer/buffer.h"

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include <string.h>

struct iceTexture_t
{
  u32 imageIndex;
  // TODO : Fix const char* directory being lost when texture vector expands
  std::string directory;

  iceTexture_t(std::string _dir) : imageIndex(0), directory(_dir) {}
};

struct IceRenderPacket
{
  glm::mat4 viewMatrix;
  glm::mat4 projectionMatrix;
};

bool WindowResizeCallback(u16 _eventCode, void* _sender, void* _listener, IceEventData _data);

class RendererBackend
{
public:
  virtual void Initialize() = 0;
  virtual void Shutdown() = 0;
  virtual void Resize(u32 _width = 0, u32 _height = 0) = 0;

  // NOTE : Virtual functions very bad for these -- WILL affect performance when called every frame
  // TODO : Find a way to override without using virtual functions
  virtual void RenderFrame(IceRenderPacket* _packet) = 0;
  virtual void RecordCommandBuffers() = 0;

  virtual void CreateDescriptorSet(u32 _programIndex) = 0;

  // TODO : DELETE
  virtual IceRenderContext* GetContext() = 0;
  virtual mesh_t CreateMesh(const char* _directory) = 0;
};

#endif // !RENDERER_RENDER_BACKEND_H
