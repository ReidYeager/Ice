
#ifndef PLATFORM_PLATFORM_H
#define PLATFORM_PLATFORM_H 1

#include "defines.h"
#include <vulkan/vulkan.h>

typedef struct PlatformState
{
  void* localState; // pointer to a platform specific struct
  b8 shouldClose;
} PlatformState;

// Creates a window
// Returns an Ice Error Code
i8 PlatformInitialize(PlatformState* _platformState, u32 _width, u32 _height, const char* _title = "IceApp");
// Destroys the window
i8 PlatformShutdown(PlatformState* _platformState);
VkSurfaceKHR PlatformCreateSurface(VkInstance* _instance);

// Allocates a block of memory on the stack of size _size
void* PlatformAllocateMem(u64 _size);
// Frees a block of memory on the stack
void  PlatformFreeMem(void* _data);
// Sets a block of memory of size _size to all 0
void* PlatformZeroMem(void* _data, u64 _size);
// Copies _size bytes from _src to _dst
void* PlatformCopyMem(void* _dst, const void* _src, u64 _size);

// Outputs a string message to the platform's console
i8 PlatformPrintToConsole(const char* message, ...);
// TODO : Research
i8 PlatformPumpMessages();

#endif // !PLATFORM_PLATFORM_H
