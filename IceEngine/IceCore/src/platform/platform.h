
#ifndef PLATFORM_PLATFORM_H
#define PLATFORM_PLATFORM_H 1

#include "defines.h"

#ifdef ICE_PLATFORM_WINDOWS
#include <windows.h>
struct LocalStateInformation
{
  HWND hwnd;
  HINSTANCE hinstance;
};
#endif // ICE_PLATFORM_WINDOWS

class Platform
{
private:
  struct PlatformStateInformation
  {
    LocalStateInformation internalState; // Should use void* instead?
    bool shouldClose;
  };
  static PlatformStateInformation platState;

public:
  Platform(u32 _width, u32 _height, const char* _title);
  ~Platform();
  bool Tick();

  static void* AllocateMem(u32 size);
  static void FreeMem(void* data);
  static void ZeroMem(void* data, u32 size);

  // TODO : Dirty. Find a better way to close
  static inline void Close() { platState.shouldClose = true; }

  // TODO : Add Logging

private:
  void PumpMessages();

};

#endif // !PLATFORM_PLATFORM_H
