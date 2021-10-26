
#ifndef ICE_RENDERER_RENDERER_BACKEND_H_
#define ICE_RENDERER_RENDERER_BACKEND_H_

#include "defines.h"
#include "logger.h"

#include "renderer/buffer.h"
#include "renderer/image.h"
#include "renderer/mesh.h"
#include "renderer/renderer_backend_context.h"
#include "renderer/vulkan/vulkan_material.h"
#include "core/ecs_components.h"
#include "core/event.h"

#include <glm/glm.hpp>
#include <string.h>
#include <vector>
#include <vulkan/vulkan.h>

// Used to pass data to the renderer
struct IceRenderPacket
{
  glm::mat4 viewMatrix;
  glm::mat4 projectionMatrix;
  float deltaTime;

  std::vector<mesh_t*> renderables;
  std::vector<u32> materialIndices;
  std::vector<TransformComponent> transforms;
};

// Calls the backend's resize function on resize events
inline bool WindowResizeCallback(u16 _eventCode, void* _sender, void* _listener, IceEventData _data);

class RendererBackend
{
public:
  virtual void Initialize()
  {
    LogDebug("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
    EventManager.Register(Ice_Event_Window_Resized, this, WindowResizeCallback);
  }

  virtual void Shutdown()
  {
    
  }


  virtual void Resize(u32 _width = 0, u32 _height = 0) = 0;

  void RenderFrame(IceRenderPacket* _packet)
  { (this->*RenderFramePointer)(_packet); }

  // NOTE : DELETE
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

bool WindowResizeCallback(u16 _eventCode, void* _sender, void* _listener, IceEventData _data)
{
  RendererBackend* rb = static_cast<RendererBackend*>(_listener);

  rb->Resize(_data.u32[0], _data.u32[1]);

  return true;
}

#endif // !RENDERER_RENDER_BACKEND_H
