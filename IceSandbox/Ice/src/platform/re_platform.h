
#ifndef ICE_PLATFORM_RE_PLATFORM_H_
#define ICE_PLATFORM_RE_PLATFORM_H_

#include "defines.h"

#include "math/vector.h"

struct reIceWindowSettings
{
  vec2 screenPosition;
  vec2 extents;
  const char* title;
};

extern class reIcePlatform
{
private:
  struct
  {
    void* vendorData = 0; // Vendor-specific information required to maintain a window
    reIceWindowSettings windowSettings = {};
    b8 shouldClose = false;
  } state;

public:
  // Initializes any components that interact with the operating system
  b8 Initialize(reIceWindowSettings* _settings);
  // Pumps the platform's queued events
  // Returns true when the window is closed
  b8 Update();
  // Destroys the window and frees its allocated memory
  b8 Shutdown();

  void Close() { state.shouldClose = true; }

  // Prints text to the platform's console
  void ConsolePrint(const char* _message, u32 _color);

  void* MemAlloc(u64 _size) { return malloc(_size); }
  void MemSet(void* _data, u64 _size, u32 _value) { memset(_data, _value, _size); }
  void MemZero(void* _data, u64 _size) { MemSet(_data, _size, 0); }
  void MemFree(void* _data) { free(_data); }

private:
  b8 CreateWindow();
  void CloseWindow();

} rePlatform;

#endif // !define ICE_PLATFORM_RE_PLATFORM_H_
