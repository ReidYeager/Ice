
#ifndef CORE_APPLICATION_H_
#define CORE_APPLICATION_H_ 1

#include "defines.h"
#include "platform/platform.h"
#include "renderer/renderer.h"
#include "renderer/shader_program.h"
#include "core/camera.h"

class ICE_API Application
{
//=================================================================================================
// Variables
//=================================================================================================
private:
  void* a = nullptr;
  void* b = nullptr;
  void* c = nullptr;

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
  //virtual void ChildInit() = 0;
  // Houses the core game code
  //virtual void ChildLoop() = 0;
  // Used for any destruction the child application requires
  //virtual void ChildShutdown() = 0;

  // TODO : Add input callbacks?

  // TODO : Modify to fit a proper ECS system
  // Temporarily : Adds the given model to the world
  void CreateObject();
  // Retrieves a shader of _name in pipeline _stagess
  void GetShaderProgramApp(const char* _name, IceShaderStageFlags _stages);

public:
  // Houses the Initialize, MainLoop, and Shutdown calls
  void Run();

  // TODO : Add any game->engine API calls here

};

#endif // !CORE_APPLICATION_H_

