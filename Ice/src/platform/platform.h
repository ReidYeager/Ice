
#ifndef ICE_PLATFORM_PLATFORM_H_
#define ICE_PLATFORM_PLATFORM_H_

#include "defines.h"

#include "math/vector.hpp"

#include <vector>

#ifdef ICE_PLATFORM_WINDOWS
#include <windows.h>
#undef CreateWindow // Remove unnecessary windows definitions
#undef CloseWindow  // Remove unnecessary windows definitions
#else
#endif // ICE_PLATFORM_WINDOWS

namespace Ice {

  //=========================
  // Memory
  //=========================
  void* MemoryAllocate(u64 _size);
  void MemorySet(void* _data, u64 _size, u32 _value);
  void MemoryCopy(void* _source, void* _destination, u64 _size);
  void MemoryFree(void* _data);

  inline void MemoryZero(void* _data, u64 _size) { MemorySet(_data, _size, 0); }
  inline void* MemoryAllocZero(u64 _size)
  {
    void* m = MemoryAllocate(_size);
    MemoryZero(m, _size);
    return m;
  }

  //=========================
  // Console
  //=========================
  void PrintToConsole(const char* _message, u32 _color);

  //=========================
  // Filesystem
  //=========================
  std::vector<char> LoadFile(const char* _directory);
  void* LoadImageFile(const char* _directory, vec2U* _extents);
  void DestroyImageFile(void* _imageData);

  //=========================
  // Window
  //=========================
  struct WindowSettings
  {
    vec2I position;
    vec2U extents;
    const char* title;
  };

  #ifdef ICE_PLATFORM_WINDOWS
  struct WindowData
  {
    HWND hwnd;
    HINSTANCE hinstance;

    WindowSettings settings;
  };
  #else
  struct WindowData
  {
    WindowSettings settings;
  };
  #endif // ICE_PLATFORM_WINDOWS

  //=========================
  // Platform data
  //=========================
  extern class Platform
  {
  private:
    WindowData window;

  public:
    b8 Update();
    b8 Shutdown();
    void CloseWindow(Ice::WindowData* _window = nullptr);

    b8 CreateNewWindow(Ice::WindowSettings _settings);
    WindowData* GetWindow() { return &window; }
  } platform;

} // namespace Ice

#endif // !ICE_PLATFORM_PLATFORM_H_
