
#include "defines.h"
#include "logger.h"

#include "platform/platform.h"
#include "core/input.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <vector>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>

//=========================
// Memory
//=========================

void* Ice::MemoryAllocate(u64 _size)
{
  return malloc(_size);
}

void Ice::MemorySet(void* _data, u64 _size, u32 _value)
{
  memset(_data, _value, _size);
}

void Ice::MemoryCopy(void* _source, void* _destination, u64 _size)
{
  memcpy(_destination, _source, _size);
}

void Ice::MemoryFree(void* _data)
{
  free(_data);
}

//=========================
// Console
//=========================

void Ice::PrintToConsole(const char* _message, u32 _color)
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

//=========================
// Filesystem
//=========================

std::vector<char> Ice::LoadFile(const char* _directory)
{
  std::ifstream inFile;
  inFile.open(_directory, std::ios::ate | std::ios::binary);
  if (!inFile)
  {
    IceLogWarning("Failed to load file\n> '%s'", _directory);
    return {};
  }

  size_t fileSize = inFile.tellg();
  inFile.seekg(0);
  std::vector<char> rawData(fileSize);
  inFile.read(rawData.data(), fileSize);

  inFile.close();
  return rawData;
}

void* Ice::LoadImageFile(const char* _directory, vec2U* _extents)
{
  IceLogInfo("Attempting to load image\n> '%s'", _directory);

  int channels;
  stbi_uc* image = stbi_load(_directory,
                             (int*)&_extents->width,
                             (int*)&_extents->height,
                             &channels,
                             STBI_rgb_alpha);
  assert (image != nullptr);
  return image;
}

void Ice::DestroyImageFile(void* _imageData)
{
  stbi_image_free(_imageData);
}

//=========================
// Platform data
//=========================

Ice::Platform Ice::platform;

b8 Ice::Platform::Update()
{
  MSG message;

  while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&message);
    DispatchMessageA(&message);
  }

  return window.hwnd != 0;
}

b8 Ice::Platform::Shutdown()
{
  CloseWindow(&window);

  return true;
}

void Ice::Platform::CloseWindow(Ice::WindowData* _window)
{
  if (_window == nullptr)
    _window = &window;

  if (_window->hwnd != 0)
  {
    DestroyWindow(_window->hwnd);
    _window->hwnd = 0;
  }
}

b8 RegisterWindow(Ice::WindowData* _data);
void PlatformAdjustWindowForBorder(Ice::WindowData* _data);
LRESULT CALLBACK ProcessInputMessage(HWND hwnd, u32 message, WPARAM wparam, LPARAM lparam);

b8 Ice::Platform::CreateNewWindow(Ice::WindowSettings _settings)
{
  window.settings = _settings;

  u32 window_style = WS_OVERLAPPEDWINDOW; //WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
  u32 window_ex_style = WS_EX_APPWINDOW;

  if (!RegisterWindow(&window))
  {
    IceLogError("Failed to register the window");
    return false;
  }
  PlatformAdjustWindowForBorder(&window);

  // Create the window
  window.hwnd = CreateWindowExA(window_ex_style,
                                "IceWindowClass",
                                window.settings.title,
                                window_style,
                                window.settings.position.x,
                                window.settings.position.y,
                                window.settings.extents.width,
                                window.settings.extents.height,
                                0,
                                0,
                                window.hinstance,
                                0);

  if (window.hwnd == 0)
  {
    IceLogFatal("Failed to create the window");
    return false;
  }

  // Show the window
  b32 shouldActivate = 1;
  i32 showWindowCommandFlags = shouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE;
  ShowWindow(window.hwnd, showWindowCommandFlags);

  return true;
}

b8 RegisterWindow(Ice::WindowData* _data)
{
  _data->hinstance = GetModuleHandleA(0);
  HICON icon = LoadIcon(_data->hinstance, IDI_APPLICATION);

  WNDCLASSA wc;
  memset(&wc, 0, sizeof(wc));
  wc.style = CS_DBLCLKS;
  wc.lpfnWndProc = ProcessInputMessage;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = _data->hinstance;
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

void PlatformAdjustWindowForBorder(Ice::WindowData* _data)
{
  u32 window_style = WS_OVERLAPPEDWINDOW; //WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
  u32 window_ex_style = WS_EX_APPWINDOW;

  RECT borderRect = { 0, 0, 0, 0 };
  AdjustWindowRectEx(&borderRect, window_style, 0, window_ex_style);

  _data->settings.position.x += borderRect.left;
  _data->settings.position.y += borderRect.top;
  _data->settings.extents.width += borderRect.right - borderRect.left;
  _data->settings.extents.height += borderRect.bottom - borderRect.top;
}

LRESULT CALLBACK ProcessInputMessage(HWND hwnd, u32 message, WPARAM wparam, LPARAM lparam)
{
  switch (message)
  {
  case WM_CLOSE:
  case WM_QUIT:
  {
    Ice::platform.CloseWindow();
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
  case WM_SIZE:
  {
    u32 width = LOWORD(lparam);
    u32 height = HIWORD(lparam);
    Ice::platform.GetWindow()->settings.extents = { width, height };
  } break;
  case WM_MOVE:
  {
    i32 x = LOWORD(lparam);
    i32 y = HIWORD(lparam);
    Ice::platform.GetWindow()->settings.position = { x, y };
  } break;
  }

  return DefWindowProcA(hwnd, message, wparam, lparam);
}
