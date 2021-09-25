
#ifndef ICE_RENDERER_RENDERER_BACKEND_H_
#define ICE_RENDERER_RENDERER_BACKEND_H_

#include "defines.h"
#include "renderer/image.h"
#include "renderer/mesh.h"
#include "renderer/buffer.h"
#include "renderer/shader_program.h"
#include "renderer/renderer_backend_context.h"
#include "renderer/vulkan/vulkan_material.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <string.h>
#include <vector>

struct IceRenderPacket
{
  glm::mat4 viewMatrix;
  glm::mat4 projectionMatrix;
  float deltaTime;

  std::vector<mesh_t*> renderables;
  IvkMaterial_T* material;
};

bool WindowResizeCallback(u16 _eventCode, void* _sender, void* _listener, IceEventData _data);

class RendererBackend
{
public:
  virtual void Initialize() = 0;
  virtual void Shutdown() = 0;
  virtual void Resize(u32 _width = 0, u32 _height = 0) = 0;

  void RenderFrame(IceRenderPacket* _packet)
  { (this->*RenderFramePointer)(_packet); }

  //void RecordCommandBuffers(IceRenderPacket* _packet)
  //{ (this->*RecordCommandBuffersPointer)(_packet); }

  virtual void CreateDescriptorSet(u32 _programIndex) = 0;

  // TODO : DELETE
  virtual IceRenderContext* GetContext() = 0;
  virtual mesh_t CreateMesh(const char* _directory) = 0;
  virtual void AddMaterial(IceMaterial _material) = 0;
  virtual iceTexture_t* GetTexture(std::string _directory) = 0;
  virtual iceImage_t* GetImage(u32 _index) = 0;

protected:
  typedef void (RendererBackend::* renderFramePtr)(IceRenderPacket* _packet);
  renderFramePtr RenderFramePointer;
  #define IceRenderBackendDefineRenderFrame(fnc) \
      RenderFramePointer = static_cast<renderFramePtr>(&##fnc)

  typedef void (RendererBackend::*RecordCommandBuffersPtr)(IvkMaterial_T* _shader);
  RecordCommandBuffersPtr RecordCommandBuffersPointer;
  #define IceRenderBackendDefineRecordCommandBuffers(fnc) \
      RecordCommandBuffersPointer = static_cast<RecordCommandBuffersPtr>(&##fnc)
};

#endif // !RENDERER_RENDER_BACKEND_H
