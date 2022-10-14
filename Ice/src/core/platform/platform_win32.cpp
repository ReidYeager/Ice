
#include "defines.h"
#include "core/platform/platform.h"
#include "core/platform/platform_defines.h"
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
  if (_size > 0)
    return malloc(_size);
  return nullptr;
}

void Ice::MemorySet(void* _data, u64 _size, u8 _value)
{
  memset(_data, _value, _size);
}

void Ice::MemoryCopy(void* _source, void* _destination, u64 _size)
{
  memcpy(_destination, _source, _size);
}

void Ice::MemoryFree(void* _data)
{
  if (_data != nullptr)
    free(_data);
}

void* Ice::MemoryReallocate(void* _data, u64 _newSize)
{
  if (_newSize > 0)
    return realloc(_data, _newSize);
  return _data;
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
  int channels;
  stbi_uc* image = stbi_load(_directory,
                             (int*)&_extents->width,
                             (int*)&_extents->height,
                             &channels,
                             STBI_rgb_alpha);
  
  ICE_ASSERT_MSG(image != nullptr, "Image not loaded\n> '%s'", _directory);
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

  return window.platformData.hwnd != 0;
}

b8 Ice::Platform::Shutdown()
{
  CloseWindow(&window);

  return true;
}

void Ice::Platform::CloseWindow(Ice::Window* _window)
{
  if (_window == nullptr)
    _window = &window;

  if (_window->platformData.hwnd != 0)
  {
    DestroyWindow(_window->platformData.hwnd);
    _window->platformData.hwnd = 0;
  }
}

b8 RegisterWindow(Ice::Window* _data);
void PlatformAdjustWindowForBorder(Ice::Window* _data);
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
  window.platformData.hwnd = CreateWindowExA(window_ex_style,
                                             "IceWindowClass",
                                             window.settings.title,
                                             window_style,
                                             window.settings.position.x,
                                             window.settings.position.y,
                                             window.settings.extents.width,
                                             window.settings.extents.height,
                                             0,
                                             0,
                                             window.platformData.hinstance,
                                             0);

  if (window.platformData.hwnd == 0)
  {
    IceLogFatal("Failed to create the window");
    return false;
  }

  #ifndef HID_USAGE_PAGE_GENERIC
  #define HID_USAGE_PAGE_GENERIC ((USHORT) 0x01)
  #endif
  #ifndef HID_USAGE_GENERIC_MOUSE
  #define HID_USAGE_GENERIC_MOUSE ((USHORT) 0x02)
  #endif

  RAWINPUTDEVICE Rid;
  Rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
  Rid.usUsage = HID_USAGE_GENERIC_MOUSE;
  Rid.dwFlags = RIDEV_INPUTSINK;
  Rid.hwndTarget = window.platformData.hwnd;
  ICE_ATTEMPT(RegisterRawInputDevices(&Rid, 1, sizeof(Rid)));
  IceLogInfo("mouse registerd");

  // Show the window
  b32 shouldActivate = 1;
  i32 showWindowCommandFlags = shouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE;
  ShowWindow(window.platformData.hwnd, showWindowCommandFlags);

  return true;
}

b8 RegisterWindow(Ice::Window* _window)
{
  _window->platformData.hinstance = GetModuleHandleA(0);
  HICON icon = LoadIcon(_window->platformData.hinstance, IDI_APPLICATION);

  WNDCLASSA wc;
  memset(&wc, 0, sizeof(wc));
  wc.style = CS_DBLCLKS;
  wc.lpfnWndProc = ProcessInputMessage;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = _window->platformData.hinstance;
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

void PlatformAdjustWindowForBorder(Ice::Window* _data)
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
    Ice::input.ProcessKeyboardKey((Ice::KeyCodeFlag)wparam, true);
  } break;
  case WM_KEYUP:
  case WM_SYSKEYUP:
  {
    Ice::input.ProcessKeyboardKey((Ice::KeyCodeFlag)wparam, false);
  } break;

  // Mouse =====
  case WM_MOUSEMOVE: // Gets the mouse's window coordinates
  {
    Ice::input.ProcessMouseWindowPos(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
  } break;
  case WM_INPUT: // Raw input (Only using mouse movement here)
  {
    // TODO : Controller input
    // https://learn.microsoft.com/en-us/windows/win32/inputdev/about-raw-input

    // Test if the input is being generated while the window is focused
    GET_RAWINPUT_CODE_WPARAM(wparam);
    if (wparam != RIM_INPUT)
      break;

    UINT size = sizeof(RAWINPUT);
    u8 data[sizeof(RAWINPUT)];
    GetRawInputData((HRAWINPUT)lparam, RID_INPUT, data, &size, sizeof(RAWINPUTHEADER));

    RAWINPUT* raw = (RAWINPUT*)data;

    if (raw->header.dwType == RIM_TYPEMOUSE)
    {
      Ice::input.ProcessMouseMove(raw->data.mouse.lLastX, raw->data.mouse.lLastY);

      // TODO : Detect if cursor is off-window when m1/m2/m3 is pressed and ignore
      //  Otherwise it locks the input value of that button to 1
      u32 buttons = (u32)raw->data.mouse.ulButtons;
      u32 index = 0;
      while (buttons != 0)
      {
        if (buttons & 3)
        {
          switch (index)
          {
          case 0: Ice::input.ProcessMouseButton(Ice::Mouse_Left   , buttons & 1); break;
          case 1: Ice::input.ProcessMouseButton(Ice::Mouse_Right  , buttons & 1); break;
          case 2: Ice::input.ProcessMouseButton(Ice::Mouse_Middle , buttons & 1); break;
          case 3: Ice::input.ProcessMouseButton(Ice::Mouse_Back   , buttons & 1); break;
          case 4: Ice::input.ProcessMouseButton(Ice::Mouse_Forward, buttons & 1); break;
          default: break;
          }
        }
        buttons = buttons >> 2;
        index++;
      }
    }
    break;
  }
  case WM_SIZE: // Resize window
  {
    u32 width = LOWORD(lparam);
    u32 height = HIWORD(lparam);
    Ice::platform.GetWindow()->settings.extents = { width, height };
  } break;
  case WM_MOVE: // Move window
  {
    i32 x = LOWORD(lparam);
    i32 y = HIWORD(lparam);
    Ice::platform.GetWindow()->settings.position = { x, y };
  } break;
  }

  return DefWindowProcA(hwnd, message, wparam, lparam);
}
