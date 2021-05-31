
#ifndef PLATFORM_PLATFORM_H
#define PLATFORM_PLATFORM_H 1

#include "defines.h"

#ifdef ICE_PLATFORM_WINDOWS
#include <windows.h>
#include <vulkan/vulkan.h>

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

  static void* AllocateMem(u32 _size);
  static void FreeMem(void* _data);
  static void ZeroMem(void* _data, u32 _size);

  static void PrintToConsole(const char* _message, ...);

  // TODO : Dirty. Find a better way to close
  static inline void Close() { platState.shouldClose = true; }

  // TODO : Find a way to create the surface without including Vulkan here
  static VkSurfaceKHR CreateSurface(VkInstance* _instance);
  static void GetWindowExtent(u32& _width, u32& _height);

private:
  void PumpMessages();

};

#endif // !PLATFORM_PLATFORM_H
