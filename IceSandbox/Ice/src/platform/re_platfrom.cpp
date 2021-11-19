
#include "defines.h"
#include "logger.h"

#include "platform/re_platform.h"
#include "core/input.h"

#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include <windowsx.h>
#undef CreateWindow // Gets rid of the windows.h preprocessor definition
#undef CloseWindow // Gets rid of the windows.h preprocessor definition

reIcePlatform rePlatform;

// Retrieves the ms-windows instance handle and defines the window's process info
b8 RegisterWindow();
// Offsets window extents to make the render area fit the input extent
void PlatformAdjustWindowForBorder(reIceWindowSettings& _window);
// Handles Windows event messages
LRESULT CALLBACK ProcessInputMessage(HWND hwnd, u32 message, WPARAM wparam, LPARAM lparam);

struct rePlatformVendorData
{
  HWND hwnd;
  HINSTANCE hinstance;
};

rePlatformVendorData* vendorData = 0;

b8 reIcePlatform::Initialize(reIceWindowSettings* _settings)
{
  if (vendorData)
  {
    IceLogError("reIcePlatform already initialized");
    return false;
  }

  state.vendorData = MemAlloc(sizeof(rePlatformVendorData));
  vendorData = (rePlatformVendorData*)state.vendorData;

  state.windowSettings = *_settings;

  if (!CreateWindow())
  {
    IceLogError("Failed to create the window");
    return false;
  }

  return true;
}

b8 reIcePlatform::Shutdown()
{
  state.shouldClose = true; // Ensure the main loop stops

  CloseWindow();

  MemFree(state.vendorData);

  return true;
}

b8 reIcePlatform::CreateWindow()
{
  reIceWindowSettings& settings = state.windowSettings;

  u32 window_style = WS_OVERLAPPEDWINDOW; //WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
  u32 window_ex_style = WS_EX_APPWINDOW;

  if (!RegisterWindow())
  {
    IceLogError("Failed to register the window");
    return false;
  }
  PlatformAdjustWindowForBorder(state.windowSettings);

  // Create the window
  HWND handle = CreateWindowExA(window_ex_style,
    "IceWindowClass",
    settings.title,
    window_style,
    settings.screenPosition.x,
    settings.screenPosition.y,
    settings.extents.width,
    settings.extents.height,
    0,
    0,
    vendorData->hinstance,
    0);

  if (handle == 0)
  {
    MessageBoxA(NULL, "Window creation failed", "Error", MB_ICONEXCLAMATION | MB_OK);
    return false;
  }
  else
  {
    vendorData->hwnd = handle;
  }

  // Show the window
  b32 shouldActivate = 1;
  i32 showWindowCommandFlags = shouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE;
  ShowWindow(vendorData->hwnd, showWindowCommandFlags);

  return true;
}

void reIcePlatform::CloseWindow()
{
  if (vendorData)
  {
    DestroyWindow(vendorData->hwnd);
    vendorData = 0;
  }
}

b8 RegisterWindow()
{
  vendorData->hinstance = GetModuleHandleA(0);
  HICON icon = LoadIcon(vendorData->hinstance, IDI_APPLICATION);

  WNDCLASSA wc;
  memset(&wc, 0, sizeof(wc));
  wc.style = CS_DBLCLKS;
  wc.lpfnWndProc = ProcessInputMessage;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = vendorData->hinstance;
  wc.hIcon = icon;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = NULL;
  wc.lpszClassName = "IceWindowClass";

  if (!RegisterClassA(&wc))
  {
    IceLogFatal("Window registration failed");
    return false;
  }

  return true;
}

void PlatformAdjustWindowForBorder(reIceWindowSettings& _window)
{
  u32 window_style = WS_OVERLAPPEDWINDOW; //WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
  u32 window_ex_style = WS_EX_APPWINDOW;

  RECT borderRect = { 0, 0, 0, 0 };
  AdjustWindowRectEx(&borderRect, window_style, 0, window_ex_style);

  _window.screenPosition.x += borderRect.left;
  _window.screenPosition.y += borderRect.top;
  _window.extents.width += borderRect.right - borderRect.left;
  _window.extents.height += borderRect.bottom - borderRect.top;
}

b8 reIcePlatform::Update()
{
  MSG message;
  while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&message);
    DispatchMessageA(&message);
  }

  return !state.shouldClose;
}

LRESULT CALLBACK ProcessInputMessage(HWND hwnd, u32 message, WPARAM wparam, LPARAM lparam)
{
  switch (message)
  {
  case WM_CLOSE:
  case WM_QUIT:
  {
    rePlatform.Close();
  } break;
  case WM_KEYDOWN:
  {
    if (wparam == Ice_Key_Escape)
      rePlatform.Close();
  } break;
  default:
    break;
  }

  return DefWindowProcA(hwnd, message, wparam, lparam);
}
