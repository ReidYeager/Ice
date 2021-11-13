
#ifndef ICE_PLATFORM_PLATFORM_H_
#define ICE_PLATFORM_PLATFORM_H_

#include "defines.h"

#include "math/vectors.h"

typedef struct IceWindow
{
  // The vendor-specific information for window creation
  void* vendorState;

  // Information required by the renderer for presentation
  void* presentationState;

  // The window's screenspace position in pixels
  vec2 position;
  // The window's width and height in pixels
  vec2 extents;
  // The name of the window's process
  const char* title;

  b8 ShouldClose;
} IceWindow;

// Allocates memory for and creates a window
// Returns : true if a window was successfully created, false otherwise
// Parameter : _window : A double pointer to set the application's platformState pointer
// Parameter : _position : The pixel position where the window shall be created
// Parameter : _extents : The pixel width and height of the desired render area
// Parameter : _title : The window's title name
b8 IcePlatformCreateWindow(void** _window,
                           vec2I _position,
                           vec2U _extents,
                           const char* _title);

// Processes the platform's input events
// Returns : true
// Parameter : _window : Not used
b8 IcePlatformUpdate(IceWindow* _window);

// Destroys the window and frees all memory
// Returns : true
// Parameter : _window : The window to destroy and free
b8 IcePlatformShutdown(IceWindow* _window);

#endif // !define ICE_PLATFORM_PLATFORM_H_
