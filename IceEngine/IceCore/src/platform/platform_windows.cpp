
#include "defines.h"
#include "platform/platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstdarg>

struct WindowsState
{
  // HWND, etc.
};

i8 Platform::Initialize()
{
  state.localState = malloc(sizeof(WindowsState));

  // TODO : Create a window
  PrintToConsole("test %d, %s", 1, "ff");
  return 0;
}

i8 Platform::Shutdown()
{
  // TODO : Destroy the window

  free(state.localState);
  return 0;
}

i8 Platform::PrintToConsole(const char* message, ...)
{
  va_list vaList;
  va_start(vaList, message);
  vprintf(message, vaList);
  va_end(vaList);

  return 0;
}

