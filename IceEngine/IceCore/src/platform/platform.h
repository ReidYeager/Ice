
#ifndef ICE_PLATFORM_PLATFORM_H_
#define ICE_PLATFORM_PLATFORM_H_

#include "defines.h"
#include "core/event.h"

#ifdef ICE_PLATFORM_WINDOWS
#include <windows.h>
// Get rid of windows.h preprocessor definition
// TODO : Make sure this is safe
#undef CreateWindow
#else
#endif

enum CursorStates
{
  Ice_Cursor_Unlocked, // Visible, not confined to window
  Ice_Cursor_Confined, // Visible, confined to window
  Ice_Cursor_Locked    // Not visible, confined to one spot
};

class IcePlatform
{
public:
  #ifdef ICE_PLATFORM_WINDOWS
  struct PlatformLocalState
  {
    HWND hwnd;
    HINSTANCE hinstance;
  };
  #else
  struct PlatformLocalState { };
  #endif

  struct PlatformStateInformation
  {
    b8 active;
    PlatformLocalState localState;
    CursorStates cursorState;

    u32 windowWidth;
    u32 windowHeight;
    u32 windowPositionX;
    u32 windowPositionY;
  } state;

public:
  void Initialize();
  void Shutdown();
  bool Update();
  void Close();

  void CreateWindow(u32 _width, u32 _height, const char* _title);
  void ResizeWindow(u32 _width, u32 _height);
  void ChangeCursorState(CursorStates _newState);

  // Size limited to 4096 MiB
  static void* AllocateMem(u32 _size);
  static void FreeMem(void* _memory);
  static void SetMem(void* _memory, u32 _size, u32 _value);
  static void ZeroMem(void* _memory, u32 _size);
  static void* CopyMem(void* _dst, void* _src, u32 _size);

  static void ConsolePrint(const char* _message, u32 _color);

};

#endif // !PLATFORM_PLATFORM_H
