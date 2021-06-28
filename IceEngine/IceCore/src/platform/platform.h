
#ifndef PLATFORM_PLATFORM_H
#define PLATFORM_PLATFORM_H 1

#include "defines.h"
#include "core/event.h"

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

enum CursorStates
{
  Ice_Cursor_Unlocked, // Visible, not confined to window
  Ice_Cursor_Confined, // Visible, confined to window
  Ice_Cursor_Locked    // Not visible, confined to one spot
};

class IcePlatform
{
private:
  struct PlatformStateInformation
  {
    LocalStateInformation internalState; // Should use void* instead?
    CursorStates cursorState;
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
  void* CopyMem(void* _dst, void* _src, u32 _size);

  void PrintToConsole(const char* _message, ...);

  void Close() { platState.shouldClose = true; }
  void ChangeCursorState(CursorStates _newState);

  // TODO : Find a way to create the surface without including Vulkan here
  VkSurfaceKHR CreateSurface(VkInstance* _instance);
  void GetWindowExtent(u32& _width, u32& _height);

private:
  void PumpMessages();

};

extern IcePlatform Platform;

inline bool PlatformClose(u16 _eventCode, void* _sender, void* _listener, IceEventData _data)
{
  static_cast<IcePlatform*>(_listener)->Close();
  return true;
}

#endif // !PLATFORM_PLATFORM_H
