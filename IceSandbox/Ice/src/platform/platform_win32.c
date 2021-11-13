
#include "defines.h"

#include "platform/platform.h"

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct VendorState
{
  HWND hwnd;
  HINSTANCE hinstance;
} VendorState;

static IceWindow* window;

// Handles queued windows input events
LRESULT CALLBACK ProcessMessage(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param);

b8 IcePlatformCreateWindow(void** _window,
                           vec2I _position,
                           vec2U _extents,
                           const char* _title)
{
  *_window = malloc(sizeof(IceWindow));
  if (!(*_window))
  {
    printf("Failed to allocate base window data\n");
    return false;
  }
  window = *_window;
  window->ShouldClose = false;

  window->vendorState = malloc(sizeof(VendorState));
  if (!window->vendorState)
  {
    printf("Failed to allocate vendor window data\n");
    return false;
  }
  VendorState* vs = window->vendorState;

  vs->hinstance = GetModuleHandleA(0);
  HICON icon = LoadIcon(vs->hinstance, IDI_APPLICATION);
  WNDCLASSA windowClass;
  memset(&windowClass, 0, sizeof(windowClass));

  windowClass.style = CS_DBLCLKS;
  windowClass.lpfnWndProc = ProcessMessage;
  windowClass.cbClsExtra = 0;
  windowClass.cbWndExtra = 0;
  windowClass.hInstance = vs->hinstance;
  windowClass.hIcon = icon;
  windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
  windowClass.hbrBackground = NULL;
  windowClass.lpszClassName = _title;

  if (!RegisterClassA(&windowClass))
  {
    printf("Failed to register MS-Windows window\n");
    return false;
  }

  u32 style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
  u32 exStyle = WS_EX_APPWINDOW;

  vec2I pos = _position;
  vec2U extents = _extents;

  RECT borders = { 0, 0, 0, 0 };
  AdjustWindowRectEx(&borders, style, 0, exStyle);

  pos.x += borders.left;
  pos.y += borders.top;

  extents.width += borders.right - borders.left;
  extents.height += borders.bottom - borders.top;

  vs->hwnd = CreateWindowExA(exStyle,
                             _title,
                             _title,
                             style,
                             pos.x,
                             pos.y,
                             extents.width,
                             extents.height,
                             0,
                             0,
                             vs->hinstance,
                             0);

  if (vs->hwnd == 0)
  {
    printf("Failed to create window\n");
    return false;
  }

  // Show the window
  ShowWindow(vs->hwnd, SW_SHOW);

  return true;
}

b8 IcePlatformUpdate(IceWindow* _window)
{
  MSG message;
  while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&message);
    DispatchMessageA(&message);
  }

  return !window->ShouldClose;
}

b8 IcePlatformShutdown(IceWindow* _window)
{
  // Destroy the window class
  DestroyWindow(((VendorState*)_window->vendorState)->hwnd);

  // Free allocated platform data
  free(_window->vendorState);
  free(_window);

  return true;
}

LRESULT CALLBACK ProcessMessage(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param)
{
  switch (msg) {
  case WM_QUIT:
  case WM_CLOSE:
    window->ShouldClose = true;
    break;
  default:
    break;
  }

  return DefWindowProcA(hwnd, msg, w_param, l_param);
}

