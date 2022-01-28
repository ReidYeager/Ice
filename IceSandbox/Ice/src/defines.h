
#ifndef ICE_DEFINES_H_
#define ICE_DEFINES_H_

#pragma warning ( disable : 26812 )
#pragma warning ( disable : 26495 )

#ifdef ICE_DLL_EXPORT
#define ICE_API
#else
#define ICE_API
#endif // ICE_DLL_EXPORT

// Comment out to disable debug-only code
#define ICE_DEBUG

//=================================================================================================
// ice primitive definitions
//=================================================================================================
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

//=================================================================================================
// Platform detection
//=================================================================================================
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define ICE_PLATFORM_WINDOWS 1
#ifndef _WIN64
#error "Only 64 bit windows supported"
#endif // !_WIN64
#else
#error "Apologies, but only MS-Windows is currently supported"
#endif // Platform detection

//=================================================================================================
// Ice Flags
//=================================================================================================
typedef u32 IceFlag;
typedef u64 IceFlagExtended;

typedef u32 IceHandle;
#define ICE_NULL_HANDLE (IceHandle)-1

//=================================================================================================
// Resource directories
//=================================================================================================
// Define these directories in a separate file
#define ICE_RESOURCE_TEXTURE_DIR "../../../Sandbox/res/textures/"
#define ICE_RESOURCE_SHADER_DIR "../../../Sandbox/res/shaders/compiled/"
#define ICE_RESOURCE_MODEL_DIR "../../../Sandbox/res/models/"

#define ICE_ATTEMPT_BOOL(x) \
if (!x)                     \
  return false;

#define ICE_ATTEMPT_HANDLE(x) \
if (x == ICE_NULL_HANDLE)     \
  return ICE_NULL_HANDLE;

#define ICE_ATTEMPT_BOOL_HANDLE(x) \
if (!x)                            \
  return ICE_NULL_HANDLE;

#define ICE_ATTEMPT_HANDLE_BOOL(x) \
if (x == ICE_NULL_HANDLE)          \
  return false;

#endif // !DEFINES_H
