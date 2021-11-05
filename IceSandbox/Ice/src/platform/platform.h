
#ifndef ICE_PLATFORM_PLATFORM_H_
#define ICE_PLATFORM_PLATFORM_H_

#include "defines.h"

#include "core/event.h"

#ifdef ICE_PLATFORM_WINDOWS
#include <windows.h>
#undef CreateWindow // Gets rid of the windows.h preprocessor definition
#else
// Non-windows platform includes
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
  // Stores platform-specific window data
  #ifdef ICE_PLATFORM_WINDOWS
  struct PlatformLocalState
  {
    HWND hwnd;
    HINSTANCE hinstance;
  };
  #else
  struct PlatformLocalState { };
  #endif

  // Stores platform generic window data
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
  // Initializes platform specific sub-layers
  void Initialize();
  // Shuts down platform specific sub-layers, destroys all active windows
  void Shutdown();
  // Handles updating platform information
  bool Update();
  // Triggers the Ice_Event_Quit event
  void Close();

  // Creates a new window
  void CreateWindow(u32 _width, u32 _height, const char* _title);
  // Changes the platform settings required to accomplish the new cursor state
  void ChangeCursorState(CursorStates _newState);

  // Dynamically allocates memory
  static void* AllocateMem(u64 _size);
  // Frees allocated memory
  static void FreeMem(void* _memory);
  // Sets an entire block of memory to _value
  static void SetMem(void* _memory, u64 _size, u32 _value);
  // Sets a block of memory to 0
  static void ZeroMem(void* _memory, u64 _size);
  // Copies data from _src to _dst
  static void* CopyMem(void* _dst, void* _src, u64 _size);

  // Prints text to the platform's console
  static void ConsolePrint(const char* _message, u32 _color);

};

#endif // !PLATFORM_PLATFORM_H
