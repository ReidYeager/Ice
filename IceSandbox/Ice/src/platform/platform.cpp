
#include "defines.h"
#include "logger.h"

#include "platform/platform.h"
#include "core/input.h"

#include "libraries/imgui/imgui.h"
#include "libraries/imgui/imgui_impl_win32.h"

#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include <windowsx.h>
#undef CreateWindow // Gets rid of the windows.h preprocessor definition
#undef CloseWindow // Gets rid of the windows.h preprocessor definition

IcePlatform rePlatform;

// Retrieves the ms-windows instance handle and defines the window's process info
b8 RegisterWindow();
// Offsets window extents to make the render area fit the input extent
void PlatformAdjustWindowForBorder(IceWindowSettings& _window);
// Handles Windows event messages
LRESULT CALLBACK ProcessInputMessage(HWND hwnd, u32 message, WPARAM wparam, LPARAM lparam);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

rePlatformVendorData* vendorData = 0;

b8 IcePlatform::Initialize(IceWindowSettings* _settings)
{
  if (vendorData)
  {
    IceLogError("IcePlatform already initialized");
    return false;
  }

  vendorData = &state.vendorData;

  state.windowSettings = *_settings;

  if (!CreateWindow())
  {
    IceLogError("Failed to create the window");
    return false;
  }

  return true;
}

b8 IcePlatform::Shutdown()
{
  state.shouldClose = true; // Ensure the main loop stops

  CloseWindow();

  return true;
}

b8 IcePlatform::CreateWindow()
{
  IceWindowSettings& settings = state.windowSettings;

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

void IcePlatform::CloseWindow()
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

void PlatformAdjustWindowForBorder(IceWindowSettings& _window)
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

b8 IcePlatform::Update()
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
  ImGui_ImplWin32_WndProcHandler(hwnd, message, wparam, lparam);

  switch (message)
  {
  case WM_CLOSE:
  case WM_QUIT:
  {
    rePlatform.Close();
  } break;

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
  default:
    break;
  }


  return DefWindowProcA(hwnd, message, wparam, lparam);
}

void IcePlatform::ConsolePrint(const char* _message, u32 _color)
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
