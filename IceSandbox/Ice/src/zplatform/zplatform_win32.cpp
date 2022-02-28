
#include "zplatform/zplatform.h"

#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <chrono>

//=========================
// Time
//=========================

// Setup time =====
std::chrono::steady_clock::time_point realtimeStart, frameStart, frameEnd;
const float microToSecond = 0.000001f;

void Ice::InitTime()
{
  realtimeStart = std::chrono::steady_clock::now();
  frameStart = frameEnd = realtimeStart;

  Ice::time.deltaTime = 0.0f;
}

void Ice::UpdateTime()
{
  frameEnd = std::chrono::steady_clock::now();

  Ice::time.deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - frameStart).count() * microToSecond;
  Ice::time.realTime = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - realtimeStart).count() * microToSecond;

  Ice::time.frameCount++;

  frameStart = frameEnd;
}

//=========================
// Memory
//=========================

void* Ice::MemoryAllocate(u64 _size)
{
  return malloc(_size);
}

void Ice::MemorySet(void* _data, u64 _size, u32 _value)
{
  memset(_data, _value, _size);
}

void Ice::MemoryCopy(void* _source, void* _destination, u64 _size)
{
  memcpy(_destination, _source, _size);
}

void Ice::MemoryFree(void* _data)
{
  free(_data);
}

//=========================
// Console
//=========================

void Ice::PrintToConsole(const char* _message, u32 _color)
{
  //               Info , Debug, Warning, Error , Fatal
  //               White, Cyan , Yellow , Red   , White-on-Red
  u32 colors[] = { 0xf  , 0xb  , 0xe    , 0x4   , 0xcf };

  HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
  SetConsoleTextAttribute(console, colors[_color]);
  OutputDebugStringA(_message);
  u64 length = strlen(_message);
  LPDWORD written = 0;
  WriteConsoleA(console, _message, (DWORD)length, written, 0);
  SetConsoleTextAttribute(console, 0xf);
}

//=========================
// File system
//=========================

std::vector<char> LoadFile(const char* _directory)
{
  return {};
}
