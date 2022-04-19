
#ifndef ICE_DEFINES_H_
#define ICE_DEFINES_H_

// These annoy me.
#pragma warning ( disable : 26812 ) // "Prefer enum-class" warning
#pragma warning ( disable : 26495 ) // "Variable is uninitialized" warning
#pragma warning ( disable : 4996 ) // "X is deprecated" warning

#ifdef _DEBUG
#define ICE_DEBUG
#endif // _DEBUG

//=========================
// Primitives
//=========================

// Integers
typedef signed char        i8;
typedef short              i16;
typedef int                i32;
typedef long long          i64;
// Unsigned Integers
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;
typedef u64 IceDeviceSize;
// Floats
typedef float              f32;
typedef double             f64;
// Booleans
typedef unsigned char      b8;
typedef unsigned int       b32;

//=========================
// Time
//=========================
namespace Ice {
  extern struct IceTime
  {
    f32 totalTime;  // Real-time in seconds since the application started
    f32 deltaTime;  // Time in seconds taken by the previous tick
    u32 frameCount; // Number of frames rendered before this tick
  } time;
}

//=========================
// Platform detection
//=========================
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define ICE_PLATFORM_WINDOWS 1
#ifndef _WIN64
#error "Only 64 bit windows supported"
#endif // !_WIN64
#else
#error "Apologies, but only MS-Windows is currently supported"
#endif // Platform detection

//=========================
// Flags
//=========================
typedef u32 IceFlag;
typedef u64 IceFlagExtended;

typedef void* IceHandle;
#define ICE_NULL_HANDLE nullptr
#define ICE_NULL_UINT -1U

//=========================
// Resource directories
//=========================
#define PREPROCESSOR_STRING(x) #x
#define EXPAND_STRING_DEF(x) PREPROCESSOR_STRING(x)

#define ICE_RESOURCE_TEXTURE_DIR EXPAND_STRING_DEF(SOURCE_DIRECTORY) "/Sandbox/res/textures/"
#define ICE_RESOURCE_SHADER_DIR EXPAND_STRING_DEF(SOURCE_DIRECTORY) "/Sandbox/res/shaders/compiled/"
#define ICE_RESOURCE_MODEL_DIR EXPAND_STRING_DEF(SOURCE_DIRECTORY) "/Sandbox/res/models/"

//=========================
// Asserts
//=========================

// Comment this out to disable asserts
#define ENABLE_ICE_ASSERTS

#ifdef ENABLE_ICE_ASSERTS

#include "logger.h"
#include <intrin.h>

#define ICE_ASSERT(expression)                                                 \
{                                                                              \
  if (!(expression))                                                           \
  {                                                                            \
    IceLogFatal("%s failed -- Line %d : %s", #expression, __LINE__, __FILE__); \
    __debugbreak();                                                            \
  }                                                                            \
}

#define ICE_ASSERT_MSG(expression, msg, ...)                                   \
{                                                                              \
  if (!(expression))                                                           \
  {                                                                            \
    IceLogFatal("%s failed -- Line %d : %s", #expression, __LINE__, __FILE__); \
    IceLogFatal(msg, __VA_ARGS__);                                             \
    __debugbreak();                                                            \
  }                                                                            \
}

#define ICE_ATTEMPT(expression) \
if (!(expression))              \
  return false;

#define ICE_ATTEMPT_MSG(expression, severity, message, ...) \
if (!(expression))                                          \
{                                                           \
  Ice::ConsoleLogMessage(severity, message, __VA_ARGS__);   \
  Ice::ConsoleLogMessage(severity, "\n");                   \
  return false;                                             \
}

#else
#define ICE_ASSERT(expression)
#endif // ENABLE_ICE_ASSERTS

#endif // !DEFINES_H
