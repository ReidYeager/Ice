
#ifndef ICE_PLATFORM_PLATFORM_DEFINES_H_
#define ICE_PLATFORM_PLATFORM_DEFINES_H_

#include "defines.h"

#include "math/vector.h"

#ifdef ICE_PLATFORM_WINDOWS
#include <windows.h>
#undef CreateWindow // Remove unnecessary windows definitions
#undef CloseWindow  // Remove unnecessary windows definitions
#endif // ICE_PLATFORM_WINDOWS

namespace Ice {

//=========================
// Window
//=========================
struct WindowSettings
{
  Ice::vec2I position;
  Ice::vec2U extents;
  const char* title;
};

struct Window
{
  // Platform specific information
  struct
  {
    #ifdef ICE_PLATFORM_WINDOWS
    HWND hwnd;
    HINSTANCE hinstance;
    #endif // ICE_PLATFORM_*
  } platformData;

  Ice::WindowSettings settings;
};

} // namespace Ice

#endif // !define ICE_PLATFORM_PLATFORM_DEFINES_H_
