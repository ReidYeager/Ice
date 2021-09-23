
#ifndef ICE_CORE_APPLICATION_H_
#define ICE_CORE_APPLICATION_H_

#include "defines.h"
#include "platform/platform.h"
#include "renderer/renderer.h"
#include "renderer/shader_program.h"
#include "core/camera.h"

#include "core/ecs_controller.h"
#include "core/gameobject.h"

class ICE_API IceApplication
{
//=================================================================================================
// Variables
//=================================================================================================
protected:
  IcePlatform* platform;
  IceRenderer* renderer;
  IceEcsController* ecsController;

public:
  // TODO : Delete
  IceCamera cam;

//=================================================================================================
// Functions
//=================================================================================================
private:
  // Initializes all allocators and subsystems
  void Initialize();
  // Loops until closed
  // Calls input, ChildLoop, and renderer
  void MainLoop();
  // Destroys all allocators and subsystems
  void Shutdown();

protected:
  // Used for any initialization the child application requires
  virtual void ChildInit() = 0;
  // Houses the core game code
  void (IceApplication::* ChildLoop)();
  #define IceApplicationDefineChildLoop(loop) \
      ChildLoop = static_cast<void (IceApplication::*)()>(&Application::##loop)
  // Used for any destruction the child application requires
  virtual void ChildShutdown() = 0;

  // TODO : Modify to fit a proper ECS system
  // Temporarily : Adds the given model to the world
  GameObject CreateObject(const char* _meshDir = nullptr);

  u32 GetMaterialIndex(std::vector<const char*> _shaderNames,
                       std::vector<IceShaderStageFlags> _shaderStages,
                       std::vector<const char*> _texStrings,
                       IceFlag _renderSettings = 0);

public:
  // Houses the Initialize, MainLoop, and Shutdown calls
  void Run();

  // TODO : Add any game->engine API calls here

};

#endif // !CORE_APPLICATION_H_

