
#include "defines.h"
#include "platform/platform.h"

#ifdef ICE_PLATFORM_WINDOWS

#include <windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

// TOOD : Remove?
#include <stdio.h>
#include <stdlib.h>
#include <cstdarg>

typedef struct WindowsState
{
  // Windows specific information
  HINSTANCE hinstance;
  HWND hwnd;
} WindowsState;

// TODO : Remove -- Only used to close the application from input callback
PlatformState* platState = nullptr;

LRESULT CALLBACK WindowsProcessInputMessage(HWND hwnd, u32 message, WPARAM wparam, LPARAM lparam)
{
  switch (message)
  {
  case WM_CLOSE:
    // TODO : Replace with a platform/application callback
    // Remove platState definition when proper callback is added
    platState->shouldClose = true;
    break;
  default:
    break;
  }

  return DefWindowProcA(hwnd, message, wparam, lparam);
}

i8 PlatformInitialize(PlatformState* _platformState, u32 _width, u32 _height, const char* _title /*= "IceApp"*/)
{
  platState = _platformState;
  _platformState->localState = malloc(sizeof(WindowsState));
  WindowsState& lstate = *((WindowsState*)_platformState->localState);


  // Register window
  lstate.hinstance = GetModuleHandleA(0);
  HICON icon = LoadIcon(lstate.hinstance, IDI_APPLICATION);
  WNDCLASSA wc;
  memset(&wc, 0, sizeof(wc)); // Replace with platform zero-memory call
  wc.style = CS_DBLCLKS; // Register double-clicks
  wc.lpfnWndProc = WindowsProcessInputMessage; // Window input event callback
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = lstate.hinstance;
  wc.hIcon = icon;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = NULL;
  wc.lpszClassName = "IceWindowClass";

  if (!RegisterClassA(&wc))
  {
    MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
    return -1;
  }

  // Resize window to account for border
  u32 client_x = 50;
  u32 client_y = 50;
  u32 client_width = _width;
  u32 client_height = _height;

  u32 window_x = client_x;
  u32 window_y = client_y;
  u32 window_width = client_width;
  u32 window_height = client_height;
  u32 window_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
  u32 window_ex_style = WS_EX_APPWINDOW;

  RECT border_rect = {0, 0, 0 , 0};
  AdjustWindowRectEx(&border_rect, window_style, 0, window_ex_style);

  window_x += border_rect.left;
  window_y += border_rect.top;

  // Create the window
  HWND handle = CreateWindowExA(window_ex_style, "IceWindowClass", _title, window_style,
      window_x, window_y, window_width, window_height, 0, 0, lstate.hinstance, 0);

  if (handle == 0)
  {
    MessageBoxA(NULL, "Window creation failed", "Error", MB_ICONEXCLAMATION | MB_OK);
    return -2;
  }
  else
  {
    lstate.hwnd = handle;
  }

  // Show the window
  b32 shouldActivate = 1;
  i32 showWindowCommandFlags = shouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE;
  ShowWindow(lstate.hwnd, showWindowCommandFlags);

  return 0;
}

i8 PlatformShutdown(PlatformState* _platformState)
{
  // TODO : Destroy the window
  WindowsState& lstate = *((WindowsState*)_platformState->localState);

  if (lstate.hwnd)
  {
    DestroyWindow(lstate.hwnd);
    lstate.hwnd = 0;
  }

  free(_platformState->localState);
  return 0;
}

VkSurfaceKHR PlatformCreateSurface(VkInstance* _instance)
{
  WindowsState& lstate = *((WindowsState*)platState->localState);

  VkWin32SurfaceCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  createInfo.flags = 0;
  createInfo.pNext = NULL;
  createInfo.hinstance = lstate.hinstance;
  createInfo.hwnd = lstate.hwnd;

  VkSurfaceKHR surface = VK_NULL_HANDLE;
  if (vkCreateWin32SurfaceKHR(*_instance, &createInfo, nullptr, &surface) != VK_SUCCESS)
  {
    // ERROR : Failed to create surface
    return nullptr;
  }

  return surface;
}

void* PlatformAllocateMem(u64 _size)
{
  return malloc(_size);
}

void  PlatformFreeMem(void* _data)
{
  free(_data);
}

void* PlatformZeroMem(void* _data, u64 _size)
{
  return memset(_data, 0, _size);
}

void* PlatformCopyMem(void* _dst, const void* _src, u64 _size)
{
  return memcpy(_dst, _src, _size);
}

i8 PlatformPrintToConsole(const char* message, ...)
{
  va_list vaList;
  va_start(vaList, message);
  vprintf(message, vaList);
  va_end(vaList);

  return 0;
}

i8 PlatformPumpMessages()
{
  MSG message;
  while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&message);
    DispatchMessageA(&message);
  }

  return 0;
}

#endif // ICE_PLATFORM_WINDOWS
