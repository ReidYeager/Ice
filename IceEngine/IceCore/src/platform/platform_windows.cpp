
#include "defines.h"

#ifdef ICE_PLATFORM_WINDOWS
#include "logger.h"
#include "platform/platform.h"
#include "core/input.h"
#include "core/event.h"

#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include <windowsx.h>
#include <fstream>
#include <vector>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

IcePlatform Platform;

// TODO : Add controller support
LRESULT CALLBACK WindowsProcessInputMessage(HWND hwnd, u32 message, WPARAM wparam, LPARAM lparam)
{
  switch (message)
  {
  case WM_CLOSE:
  {
    Platform.Close();
  } break;
  case WM_SIZE:
  {
    RECT winRect;
    GetWindowRect(hwnd, &winRect);

    IceEventData data {};
    data.u32[0] = winRect.right - winRect.left;
    data.u32[1] = winRect.bottom - winRect.top;

    EventManager.Fire(Ice_Event_Window_Resized, nullptr, data);
  } break;
  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
  case WM_KEYUP:
  case WM_SYSKEYUP:
  {
    b8 pressed = (message == WM_KEYDOWN);
    u32 key = (u32)wparam;
    Input.ProcessKeyboardKey(key, pressed);
    IceEventData data {};
    data.u32[0] = (IceKeyCodeFlag)key;

    if (message == WM_KEYUP || message == WM_SYSKEYUP)
    {
      EventManager.Fire(Ice_Event_Key_Released, nullptr, data);
    }
    else
    {
      EventManager.Fire(Ice_Event_Key_Pressed, nullptr, data);
    }
  } break;
  case WM_MOUSEMOVE:
  {
    // TODO : Add raw input support
    //  https://docs.microsoft.com/en-us/windows/win32/inputdev/raw-input?redirectedfrom=MSDN
    i32 x = GET_X_LPARAM(lparam);
    i32 y = GET_Y_LPARAM(lparam);
    Input.ProcessMouseMove(x, y);

    IceEventData data {};
    Input.GetMouseDelta(&data.i32[0], &data.i32[1]);
    EventManager.Fire(Ice_Event_Mouse_Moved, nullptr, data);
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
    b8 pressed = message == WM_LBUTTONDOWN
              || message == WM_RBUTTONDOWN
              || message == WM_MBUTTONDOWN;

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
      u32 xbutton = GET_XBUTTON_WPARAM(wparam);
      switch (xbutton)
      {
      case 1:
        button = Ice_Mouse_Back; break;
      case 2:
        button = Ice_Mouse_Forward; break;
      default:
        button = Ice_Mouse_Extra; break;
      }
    }
      break;
    }

    if (button != Ice_Mouse_Max)
    {
      Input.ProcessMouseButton(button, pressed);

      IceEventCodes code =
          pressed ? Ice_Event_Mouse_Button_Pressed : Ice_Event_Mouse_Button_Released;
      IceEventData data {};
      data.u32[0] = button;

      EventManager.Fire(code, nullptr, data);
    }
  } break;
  default:
    break;
  }

  return DefWindowProcA(hwnd, message, wparam, lparam);
}

void IcePlatform::Initialize(u32 _width, u32 _height, const char* _title)
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
  u32 window_style = WS_OVERLAPPEDWINDOW; //WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
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

  IcePrint("Initialized Platform system");

  Input.Initialize();
}

void IcePlatform::Shutdown()
{
  Input.Shutdown();

  if (platState.internalState.hwnd)
  {
    DestroyWindow(platState.internalState.hwnd);
    platState.internalState.hwnd = 0;
  }

  IcePrint("Shutdown Platform system");
}

bool IcePlatform::Tick()
{
  PumpMessages();

  return !platState.shouldClose;
}

void* IcePlatform::AllocateMem(u32 _size)
{
  return malloc(_size);
}

void IcePlatform::FreeMem(void* _data)
{
  free(_data);
}

void IcePlatform::ZeroMem(void* _data, u32 _size)
{
  memset(_data, 0, _size);
}

void* IcePlatform::CopyMem(void* _dst, void* _src, u32 _size)
{
  return memcpy(_dst, _src, _size);
}

void IcePlatform::PrintToConsole(const char* _message, ...)
{
  va_list args;
  va_start(args, _message);
  vprintf(_message, args);
  va_end(args);
}

void IcePlatform::Close()
{
  platState.shouldClose = true;
  EventManager.Fire(Ice_Event_Quit, nullptr, {});
}

bool CursorResetCallback(u16 _eventCode, void* _sender, void* _listener, IceEventData _data)
{
  //SetCursorPos(150, 150);
  //IcePrint("(%d, %d)", _data.i32[0], _data.i32[1]);
  return true;
}

void IcePlatform::ChangeCursorState(CursorStates _newState)
{
  //if (_newState == platState.cursorState)
  //  return;
  //if (platState.cursorState == Ice_Cursor_Locked)
  //  EventManager.Unregister(Ice_Event_Mouse_Moved, this, CursorResetCallback);

  //switch (_newState)
  //{
  //case Ice_Cursor_Unlocked:
  //{
  //  ShowCursor(true);
  //} break;
  //case Ice_Cursor_Confined:
  //{
  //  RECT windowRect;
  //  GetWindowRect(platState.internalState.hwnd, &windowRect);
  //  ClipCursor(&windowRect);
  //  ShowCursor(true);
  //} break;
  //case Ice_Cursor_Locked:
  //{
  //  SetCursorPos(150, 150);
  //  ShowCursor(false);
  //  EventManager.Register(Ice_Event_Mouse_Moved, this, CursorResetCallback);
  //} break;
  //} // End switch

  //platState.cursorState = _newState;
}

VkSurfaceKHR IcePlatform::CreateSurface(VkInstance* _instance)
{
  VkWin32SurfaceCreateInfoKHR createInfo {};
  createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  createInfo.hinstance = platState.internalState.hinstance;
  createInfo.hwnd = platState.internalState.hwnd;
  createInfo.flags = 0;

  VkSurfaceKHR surface;
  if (vkCreateWin32SurfaceKHR(*_instance, &createInfo, nullptr, &surface) != VK_SUCCESS)
  {
    IcePrint("Failed to create vkSurface\n");
    return VK_NULL_HANDLE;
  }

  return surface;
}

void IcePlatform::GetWindowExtent(u32& _width, u32& _height)
{
  RECT rect;

  if (!GetWindowRect(platState.internalState.hwnd, &rect))
  {
    IcePrint("Failed to get window rect");
  }

  _width = rect.right - rect.left;
  _height = rect.bottom - rect.top;
}

void IcePlatform::PumpMessages()
{
  MSG message;
  while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&message);
    DispatchMessageA(&message);
  }
}

#endif // ENG_PLATFORM_WINDOWS_32
