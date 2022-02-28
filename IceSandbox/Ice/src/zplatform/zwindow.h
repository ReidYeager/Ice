
#ifndef ICE_ZPLATFORM_ZWINDOW_H_
#define ICE_ZPLATFORM_ZWINDOW_H_

#include "defines.h"

#include "math/vector.h"

struct zIceWindowSettings
{
  const char* title = "Ice window";
  vec2U extents = {255, 255};
  vec2I position = {50, 50};

  // Other statuses (fullscreen, vsync, hdr?, etc)
};

#ifdef ICE_PLATFORM_WINDOWS
#include <windows.h>
struct zIceWindowVendorData
{
  HWND hwnd;
  HINSTANCE hinstance;
};
#else
struct zIceWindowVendorData {};
#endif // ICE_PLATFORM_WINDOWS

class zIceWindow
{
private:
  zIceWindowVendorData vendorData;
  b8 active;

public:
  b8 Initialize(zIceWindowSettings _settings);
  b8 SetVisiblity(b8 _isVisible);
  b8 Update();
  void Close();
};

#endif // !define ICE_ZPLATFORM_ZWINDOW_H_
