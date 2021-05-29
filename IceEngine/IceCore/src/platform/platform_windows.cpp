
#include "defines.h"

#ifdef ICE_PLATFORM_WINDOWS
#include "platform/platform.h"

#include <windows.h>
#include <vulkan/vulkan.h>

Platform::PlatformStateInformation Platform::platState;

LRESULT CALLBACK WindowsProcessInputMessage(HWND hwnd, u32 message, WPARAM wparam, LPARAM lparam)
{
  switch (message)
  {
  case WM_CLOSE:
    // Callback to platform
    // TODO : Dirty. Find a better way to close
    Platform::Close();
    break;
  default:
    break;
  }

  return DefWindowProcA(hwnd, message, wparam, lparam);
}

Platform::Platform(u32 _width, u32 _height, const char* _title)
{
  ZeroMem(&platState, sizeof(PlatformStateInformation));
  LocalStateInformation& lstate = platState.internalState;

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
    return;
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

  RECT border_rect = { 0, 0, 0 , 0 };
  AdjustWindowRectEx(&border_rect, window_style, 0, window_ex_style);

  window_x += border_rect.left;
  window_y += border_rect.top;

  // Create the window
  HWND handle = CreateWindowExA(window_ex_style, "IceWindowClass", _title, window_style,
    window_x, window_y, window_width, window_height, 0, 0, lstate.hinstance, 0);

  if (handle == 0)
  {
    MessageBoxA(NULL, "Window creation failed", "Error", MB_ICONEXCLAMATION | MB_OK);
    return;
  }
  else
  {
    lstate.hwnd = handle;
  }

  // Show the window
  b32 shouldActivate = 1;
  i32 showWindowCommandFlags = shouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE;
  ShowWindow(lstate.hwnd, showWindowCommandFlags);
}

Platform::~Platform()
{
  if (platState.internalState.hwnd)
  {
    DestroyWindow(platState.internalState.hwnd);
    platState.internalState.hwnd = 0;
  }
}

bool Platform::Tick()
{
  PumpMessages();

  return !platState.shouldClose;
}

void* Platform::AllocateMem(u32 size)
{
  return malloc(size);
}

void Platform::FreeMem(void* data)
{
  free(data);
}

void Platform::ZeroMem(void* data, u32 size)
{
  memset(data, 0, size);
}

void Platform::PumpMessages()
{
  MSG message;
  while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&message);
    DispatchMessageA(&message);
  }
}

#endif // ENG_PLATFORM_WINDOWS_32
