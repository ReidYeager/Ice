
#ifndef CORE_APPLICATION_H_
#define CORE_APPLICATION_H_ 1

#include "defines.h"

class ICE_API Application
{
//=================================================================================================
// Variables
//=================================================================================================
private:
  b8 m_shouldClose = false;

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

