
#ifndef ICE_CORE_APPLICATION_H_
#define ICE_CORE_APPLICATION_H_

#include "defines.h"

#include "core/camera.h"
#include "core/ecs_controller.h"
#include "core/ecs_components.h"
#include "core/gameobject.h"
#include "platform/platform.h"
#include "renderer/renderer.h"
#include "renderer/shader.h"

class ICE_API IceApplication
{
//=================================================================================================
// Variables
//=================================================================================================
private:
  IceRenderPacket renderPacket;

protected:
  IcePlatform* platform;
  IceRenderer* renderer;
  IceEcsController* ecsController;

public:
  // NOTE : Delete
  IceCamera cam;

//=================================================================================================
// Functions
//=================================================================================================
private:
  // Used for any initialization the child application requires
  void(*gameInit)() = 0;
  // Houses the core game code
  void(*gameLoop)(float) = 0;
  // Used for any destruction the child application requires
  void(*gameShutdown)() = 0;

  // Initializes all allocators and subsystems
  void Startup();
  // Loops until closed
  // Calls input, ChildLoop, and renderer
  void MainLoop();
  // Destroys all allocators and subsystems
  void Shutdown();
  // Called when RenderableComponents are added or removed from objects
  void RenderableCallback();

public:
  void Initialize(void(*_gameInit)(), void(*_gameLoop)(float), void(*_gameShutdown)());
  // Houses the Initialize, MainLoop, and Shutdown calls
  void Run();

  // Tells the application to close
  void Close();

  // Creates a new ecs entity and returns a wrapper for it
  GameObject CreateObject();
  // Load or retrieve the mesh index of the provided directory
  u32 GetMeshIndex(const char* _meshDir = nullptr);
  // Load or retrieve the index of the material with the provided shaders and settings
  u32 GetMaterialIndex(
      std::vector<const char*> _shaderNames,          // The shaders' directory file names
      std::vector<IceShaderStageFlags> _shaderStages, // The stage(s) to use each shader
      std::vector<const char*> _texStrings,           // Texture directories to use (if any)
      IceFlag _renderSettings = 0);                   // Render pipeline settings
  void MaterialUpdateBuffer(u32 _material,
                            IceShaderStageFlags _stage,
                            IceShaderBufferParameterFlags _userParameterFlags,
                            void* _userData);
  void MaterialUpdateTextures(u32 _material,
                              std::vector<const char*> _textureNames);
};

#endif // !CORE_APPLICATION_H_

