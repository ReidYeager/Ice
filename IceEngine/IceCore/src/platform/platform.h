
#ifndef PLATFORM_PLATFORM_H
#define PLATFORM_PLATFORM_H 1

#include "defines.h"

class Platform
{
//=================================================================================================
// Variables
//=================================================================================================
private:
  struct PlatformState
  {
    void* localState; // pointer to a platform specific struct
  } state;

//=================================================================================================
// Functions
//=================================================================================================
public:
  // Creates a window
  // Returns an Ice Error Code
  i8 Initialize(u32 _width, u32 _height, const char* _title = "IceApp");
  // Destroys the window
  i8 Shutdown();

  // Outputs a string message to the platform's console
  i8 PrintToConsole(const char* message, ...);

  // TODO : Research
  i8 PlatformPumpMessages();

};

#endif // !PLATFORM_PLATFORM_H
