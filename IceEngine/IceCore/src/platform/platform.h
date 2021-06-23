
#ifndef PLATFORM_PLATFORM_H
#define PLATFORM_PLATFORM_H 1

#include "defines.h"

#ifdef ICE_PLATFORM_WINDOWS
#include <windows.h>
#include <vulkan/vulkan.h>

#include <vector>

struct LocalStateInformation
{
  HWND hwnd;
  HINSTANCE hinstance;
};
#endif // ICE_PLATFORM_WINDOWS

class IcePlatform
{
private:
  struct PlatformStateInformation
  {
    LocalStateInformation internalState; // Should use void* instead?
    bool shouldClose;
  };
  PlatformStateInformation platState;

public:
  
  void Initialize(u32 _width, u32 _height, const char* _title);
  void Shutdown();
  bool Tick();

  void* AllocateMem(u32 _size);
  void FreeMem(void* _data);
  void ZeroMem(void* _data, u32 _size);

  void PrintToConsole(const char* _message, ...);

  // TODO : Dirty. Find a better way to close
  void Close() { platState.shouldClose = true; }

  // TODO : Find a way to create the surface without including Vulkan here
  VkSurfaceKHR CreateSurface(VkInstance* _instance);
  void GetWindowExtent(u32& _width, u32& _height);

private:
  void PumpMessages();

};

extern IcePlatform Platform;

#endif // !PLATFORM_PLATFORM_H
