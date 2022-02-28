
#ifndef ICE_ZPLATFORM_ZPLATFORM_H_
#define ICE_ZPLATFORM_ZPLATFORM_H_

#include "defines.h"

#include "zplatform/zwindow.h"

#include <vector>

namespace Ice
{
  //=========================
  // Time
  //=========================
  extern struct IceTime
  {
    f64 realTime = 0.0; // Real-time in seconds since the application started
    f32 deltaTime = 0.0f; // Time in seconds taken by the previous tick

    u64 frameCount = 0;
  } time;
  void InitTime();
  void UpdateTime();

  //=========================
  // Memory
  //=========================
  void* MemoryAllocate(u64 _size);
  void MemorySet(void* _data, u64 _size, u32 _value);
  void MemoryCopy(void* _source, void* _destination, u64 _size);
  void MemoryFree(void* _data);

  inline void MemoryZero(void* _data, u64 _size) { MemorySet(_data, _size, 0); }

  //=========================
  // Console
  //=========================
  void PrintToConsole(const char* _message, u32 _color);

  //=========================
  // File system
  //=========================
  std::vector<char> LoadFile(const char* _directory);

  //=========================
  // Window
  //=========================
  inline zIceWindow CreateNewWindow(zIceWindowSettings _settings)
  {
    zIceWindow window;
    window.Initialize(_settings);
    return window;
  }

}

#endif // !ICE_ZPLATFORM_ZPLATFORM_H_
