
#include "defines.h"
#include "logger.h"

#include "zplatform/zwindow.h"

#include "zplatform/zplatform.h"
#include "core/input.h"

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

  vec2U windowExtents = _settings.extents;
  vec2I windowPosition = _settings.position;

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

    windowPosition.x += borderRect.left;
    windowPosition.y += borderRect.top;
    windowExtents.width += borderRect.right - borderRect.left;
    windowExtents.height += borderRect.bottom - borderRect.top;
  }
  IceLogInfo("Window using (%u, %u) at (%d, %d)",
             windowExtents.width,
             windowExtents.height,
             windowPosition.x,
             windowPosition.y);

  // Create window =====
  {
    vendorData.hwnd = CreateWindowExA(windowExStyle,
                                      windowClassName,
                                      _settings.title,
                                      windowStyle,
                                      windowPosition.x,
                                      windowPosition.y,
                                      windowExtents.width,
                                      windowExtents.height,
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

  // Keyboard =====
  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
  {
    Input.ProcessKeyboardKey(wparam, true);
  } break;
  case WM_KEYUP:
  case WM_SYSKEYUP:
  {
    Input.ProcessKeyboardKey(wparam, false);
  } break;

  // Mouse =====
  case WM_MOUSEMOVE:
  {
    Input.ProcessMouseMove(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
  } break;
  case WM_LBUTTONDOWN:
  case WM_LBUTTONUP:
  case WM_RBUTTONDOWN:
  case WM_RBUTTONUP:
  case WM_MBUTTONDOWN:
  case WM_MBUTTONUP:
  case WM_XBUTTONDOWN:
  case WM_XBUTTONUP:
  {
    b8 pressed = message == WM_LBUTTONDOWN ||
                 message == WM_RBUTTONDOWN ||
                 message == WM_MBUTTONDOWN ||
                 message == WM_XBUTTONDOWN;

    IceMouseButtonFlag button = Ice_Mouse_Max;
    switch (message)
    {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
      button = Ice_Mouse_Left; break;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
      button = Ice_Mouse_Right; break;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
      button = Ice_Mouse_Middle; break;
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    {
      u32 xButton = GET_XBUTTON_WPARAM(wparam);
      switch (xButton)
      {
        case 1: button = Ice_Mouse_Back; break;
        case 2: button = Ice_Mouse_Forward; break;
        default: button = Ice_Mouse_Extra; break;
      }
    } break;
    default:
      break;
    }

    if (button != Ice_Mouse_Max)
    {
      Input.ProcessMouseButton(button, pressed);
    }
  } break;
  //case WM_SIZE:
  //{
  //  u32 width = LOWORD(lparam);
  //  u32 height = HIWORD(lparam);
  //  //extents = { width, height };
  //} break;
  default:
    return DefWindowProcA(hwnd, message, wparam, lparam);
  }
}

