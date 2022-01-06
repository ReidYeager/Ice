
#ifndef ICE_PLATFORM_RE_PLATFORM_H_
#define ICE_PLATFORM_RE_PLATFORM_H_

#include "defines.h"

#include "math/vector.h"

struct IceWindowSettings
{
  vec2I screenPosition;
  vec2U extents;
  const char* title;
};

// Vendor-specific information required to maintain a window
#ifdef ICE_PLATFORM_WINDOWS
#include <Windows.h>
#undef CreateWindow // Gets rid of the windows.h preprocessor definition
#undef CloseWindow // Gets rid of the windows.h preprocessor definition
struct rePlatformVendorData
{
  HWND hwnd;
  HINSTANCE hinstance;
};
#else
struct rePlatformVendorData {};
#endif // ICE_PLATFORM_WINDOWS

extern class IcePlatform
{
private:
  struct
  {
    rePlatformVendorData vendorData;
    IceWindowSettings windowSettings = {};
    b8 shouldClose = false;
  } state;

public:
  // Initializes any components that interact with the operating system
  b8 Initialize(IceWindowSettings* _settings);
  // Pumps the platform's queued events
  // Returns true when the window is closed
  b8 Update();
  // Destroys the window and frees its allocated memory
  b8 Shutdown();

  void Close() { state.shouldClose = true; }

  rePlatformVendorData const* GetVendorInfo() { return &state.vendorData; }
  IceWindowSettings const* GetWindowInfo() { return &state.windowSettings; }

  // Prints text to the platform's console
  void ConsolePrint(const char* _message, u32 _color);

  void* MemAlloc(u64 _size) { return malloc(_size); }
  void MemSet(void* _data, u64 _size, u32 _value) { memset(_data, _value, _size); }
  void MemZero(void* _data, u64 _size) { MemSet(_data, _size, 0); }
  void MemFree(void* _data) { free(_data); }
  void MemCopy(void* _src, void* _dst, u64 _size) { memcpy(_dst, _src, _size); }

private:
  b8 CreateWindow();
  void CloseWindow();

} rePlatform;

#endif // !define ICE_PLATFORM_RE_PLATFORM_H_
