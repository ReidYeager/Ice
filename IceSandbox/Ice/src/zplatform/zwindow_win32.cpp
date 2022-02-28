
#include "defines.h"
#include "logger.h"

#include "zplatform/zwindow.h"

#include "zplatform/zplatform.h"

#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include <windowsx.h>
#undef CreateWindow // Gets rid of the windows.h preprocessor definition
#undef CloseWindow // Gets rid of the windows.h preprocessor definition

LRESULT CALLBACK zProcessInputMessage(HWND hwnd, u32 message, WPARAM wparam, LPARAM lparam);

zIceWindow* inputWindow;

b8 zIceWindow::Initialize(zIceWindowSettings _settings)
{
  u32 windowStyle = WS_OVERLAPPEDWINDOW;
  u32 windowExStyle = WS_EX_APPWINDOW;
  const char* windowClassName = "IceApplicationWindowClass";

  vec2U extents = _settings.extents;
  vec2I position = _settings.position;

  // Register window =====
  {
    vendorData.hinstance = GetModuleHandleA(0);

    WNDCLASSA wc;
    Ice::MemorySet(&wc, sizeof(wc), 0);
    wc.style = CS_DBLCLKS; // Enable double-click
    wc.lpfnWndProc = zProcessInputMessage;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = vendorData.hinstance;
    wc.hIcon = LoadIcon(vendorData.hinstance, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = windowClassName;

    if (!RegisterClassA(&wc))
    {
      IceLogFatal("Failed to register the Win32 window");
      return false;
    }
  }

  // Resize for canvas =====
  {
    RECT borderRect = {0, 0, 0, 0};
    AdjustWindowRectEx(&borderRect, windowStyle, 0, windowExStyle);

    position.x += borderRect.left;
    position.y += borderRect.top;
    extents.width += borderRect.right - borderRect.left;
    extents.height += borderRect.bottom - borderRect.top;
  }
  IceLogInfo("Window using (%u, %u) at (%d, %d)",
             extents.width,
             extents.height,
             position.x,
             position.y);

  // Create window =====
  {
    vendorData.hwnd = CreateWindowExA(windowExStyle,
                                      windowClassName,
                                      _settings.title,
                                      windowStyle,
                                      position.x,
                                      position.y,
                                      extents.width,
                                      extents.height,
                                      0,
                                      0,
                                      vendorData.hinstance,
                                      0);

    if (!vendorData.hwnd)
    {
      IceLogFatal("Failed to create Win32 window");
      return false;
    }
  }

  if (!SetVisiblity(true))
    return false;

  active = true;

  return true;
}

b8 zIceWindow::SetVisiblity(b8 _isVisible)
{
  i32 commandFlags = _isVisible ? SW_SHOW : SW_HIDE;

  ShowWindow(vendorData.hwnd, commandFlags);

  return true;
}

b8 zIceWindow::Update()
{
  inputWindow = this;

  MSG message;

  while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&message);
    DispatchMessageA(&message);
  }

  return active;
}

void zIceWindow::Close()
{
  if (active)
  {
    DestroyWindow(vendorData.hwnd);
    Ice::MemoryZero(&vendorData, sizeof(vendorData));
    active = false;
  }
}

LRESULT CALLBACK zProcessInputMessage(HWND hwnd, u32 message, WPARAM wparam, LPARAM lparam)
{
  switch (message)
  {
  case WM_CLOSE:
  case WM_QUIT:
  {
    inputWindow->Close();
    return false;
  }
  default:
    return DefWindowProcA(hwnd, message, wparam, lparam);
  }
}

