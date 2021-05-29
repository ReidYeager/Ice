
#ifndef CORE_APPLICATION_H_
#define CORE_APPLICATION_H_ 1

#include "defines.h"
#include "platform/platform.h"
#include "renderer/renderer.h"

class ICE_API Application
{
//=================================================================================================
// Variables
//=================================================================================================
private:
  Platform* platform = nullptr;
  Renderer* renderer = nullptr;

  void* a = nullptr;
  void* b = nullptr;
  void* c = nullptr;

//=================================================================================================
// Functions
//=================================================================================================
private:
  // Initializes all allocators and subsystems
  i8 Initialize();
  // Loops until closed
  // Calls input, ChildLoop, and renderer
  i8 MainLoop();
  // Destroys all allocators and subsystems
  i8 Shutdown();


protected:
  //// Used for any initialization the child application requires
  //virtual void ChildInit() = 0;
  //// Houses the core game code
  //virtual void ChildLoop() = 0;
  //// Used for any destruction the child application requires
  //virtual void ChildShutdown() = 0;

public:
  // Houses the Initialize, MainLoop, and Shutdown calls
  void Run();

  // TODO : Add any game->engine API calls here

};

#endif // !CORE_APPLICATION_H_

