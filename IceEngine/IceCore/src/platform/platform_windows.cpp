
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

// Offsets window extents to make the render area fit the input extent
void PlatformAdjustWindowForBorder(IcePlatform* _platform);
// Registers the window with Windows
void PlatformRegisterWindow(IcePlatform* _platform);
// Handles Windows event messages
LRESULT CALLBACK WindowsProcessInputMessage(HWND hwnd, u32 message, WPARAM wparam, LPARAM lparam);

// Ice callback to allow escape from the main loop
bool QuitEventCallback(u16 _eventCode, void* _sender, void* _listener, IceEventData _data)
{
  ((IcePlatform*)_listener)->state.active = false;
  return true;
}

void IcePlatform::Initialize()
{
  // Ensure singleton
  if (state.active)
  {
    LogDebug("Platform already initialized");
    return;
  }

  ZeroMem(&state, sizeof(PlatformStateInformation));
  EventManager.Register(Ice_Event_Quit, this, QuitEventCallback);
  state.active = true;

  LogInfo("Initialized Platform system");

  Input.Initialize();
}

void IcePlatform::Shutdown()
{
  Input.Shutdown();

  // Destroy remaining windows
  if (state.localState.hwnd)
  {
    DestroyWindow(state.localState.hwnd);
    state.localState.hwnd = 0;
  }

  LogInfo("Shutdown Platform system");
}

void IcePlatform::CreateWindow(u32 _width, u32 _height, const char* _title)
{
  PlatformLocalState& lstate = state.localState;

  // Resize window to account for border
  state.windowPositionX = 50;
  state.windowPositionY = 50;
  state.windowWidth = _width;
  state.windowHeight = _height;

  u32 window_style = WS_OVERLAPPEDWINDOW; //WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
  u32 window_ex_style = WS_EX_APPWINDOW;

  PlatformRegisterWindow(this);
  PlatformAdjustWindowForBorder(this);

  // Create the window
  HWND handle = CreateWindowExA(window_ex_style,
                                "IceWindowClass",
                                _title,
                                window_style,
                                state.windowPositionX,
                                state.windowPositionY,
                                state.windowWidth,
                                state.windowHeight,
                                0,
                                0,
                                lstate.hinstance,
                                0);

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

void PlatformRegisterWindow(IcePlatform* _platform)
{
  IcePlatform::PlatformLocalState* state = &_platform->state.localState;

  state->hinstance = GetModuleHandleA(0);
  HICON icon = LoadIcon(state->hinstance, IDI_APPLICATION);

  WNDCLASSA wc;
  _platform->ZeroMem(&wc, sizeof(wc));
  wc.style = CS_DBLCLKS;
  wc.lpfnWndProc = WindowsProcessInputMessage;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = state->hinstance;
  wc.hIcon = icon;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = NULL;
  wc.lpszClassName = "IceWindowClass";

  if (!RegisterClassA(&wc))
  {
    MessageBoxA(0, "Ice window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
  }
}

void PlatformAdjustWindowForBorder(IcePlatform* _platform)
{
  IcePlatform::PlatformStateInformation* state = &_platform->state;
  u32 window_style = WS_OVERLAPPEDWINDOW; //WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
  u32 window_ex_style = WS_EX_APPWINDOW;

  RECT borderRect = { 0, 0, 0, 0 };
  AdjustWindowRectEx(&borderRect, window_style, 0, window_ex_style);

  state->windowPositionX += borderRect.left;
  state->windowPositionY += borderRect.top;
  state->windowWidth += borderRect.right - borderRect.left;
  state->windowHeight += borderRect.bottom - borderRect.top;
}

bool IcePlatform::Update()
{
  MSG message;
  while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&message);
    DispatchMessageA(&message);
  }

  return state.active;
}

void* IcePlatform::AllocateMem(u64 _size)
{
  return malloc(_size);
}

void IcePlatform::FreeMem(void* _data)
{
  free(_data);
}

void IcePlatform::SetMem(void* _memory, u64 _size, u64 _value)
{
  memset(_memory, _value, _size);
}

void IcePlatform::ZeroMem(void* _data, u64 _size)
{
  memset(_data, 0, _size);
}

void* IcePlatform::CopyMem(void* _dst, void* _src, u64 _size)
{
  return memcpy(_dst, _src, _size);
}

void IcePlatform::ConsolePrint(const char* _message, u32 _color)
{
  //               Info , Debug, Error , Fatal
  //               White, Cyan , Yellow, White-on-Red
  u32 colors[] = { 0xf  , 0xb  , 0xe   , 0xcf };

  HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
  SetConsoleTextAttribute(console, colors[_color]);
  OutputDebugStringA(_message);
  u64 length = strlen(_message);
  LPDWORD written = 0;
  WriteConsoleA(console, _message, (DWORD)length, written, 0);
  SetConsoleTextAttribute(console, 0xf);
}

void IcePlatform::Close()
{
  state.active = false;
  EventManager.Fire(Ice_Event_Quit, nullptr, {});
}

bool CursorResetCallback(u16 _eventCode, void* _sender, void* _listener, IceEventData _data)
{
  //SetCursorPos(150, 150);
  //LogInfo("(%d, %d)", _data.i32[0], _data.i32[1]);
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

LRESULT CALLBACK WindowsProcessInputMessage(HWND hwnd, u32 message, WPARAM wparam, LPARAM lparam)
{
  switch (message)
  {
  case WM_CLOSE:
  {
    EventManager.Fire(Ice_Event_Quit, nullptr, {});
  } break;
  case WM_SIZE:
  {
    RECT winRect;
    GetWindowRect(hwnd, &winRect);

    IceEventData data{};
    data.u32[0] = winRect.right - winRect.left;
    data.u32[1] = winRect.bottom - winRect.top;

    EventManager.Fire(Ice_Event_Window_Resized, nullptr, data);
  } break;
  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
  case WM_KEYUP:
  case WM_SYSKEYUP:
  {
    b8 pressed = 0;
    u32 key = (u32)wparam;

    pressed = (message == WM_KEYDOWN);
    Input.ProcessKeyboardKey(key, pressed);

    // Using pressed/released flags prevents hitching from key press events

    //IceEventData data{};
    //data.u32[0] = (IceKeyCodeFlag)key;
    //if (message == WM_KEYUP || message == WM_SYSKEYUP)
    //{
    //  EventManager.Fire(Ice_Event_Key_Released, nullptr, data);
    //}
    //else
    //{
    //  EventManager.Fire(Ice_Event_Key_Pressed, nullptr, data);
    //}
  } break;
  case WM_MOUSEMOVE:
  {
    // Raw input support
    // https://docs.microsoft.com/en-us/windows/win32/inputdev/raw-input?redirectedfrom=MSDN
    i32 x = GET_X_LPARAM(lparam);
    i32 y = GET_Y_LPARAM(lparam);
    Input.ProcessMouseMove(x, y);
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
      IceEventData data{};
      data.u32[0] = button;

      EventManager.Fire(code, nullptr, data);
    }
  } break;
  default:
    break;
  }

  return DefWindowProcA(hwnd, message, wparam, lparam);
}


#endif // ENG_PLATFORM_WINDOWS_32
