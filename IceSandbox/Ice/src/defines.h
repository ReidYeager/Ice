
#ifndef ICE_DEFINES_H_
#define ICE_DEFINES_H_

//#pragma warning ( disable : 26812 )
//#pragma warning ( disable : 26495 )

#ifdef ICE_DLL_EXPORT
#define ICE_API
#else
#define ICE_API extern "C"
#endif

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
// Floats
typedef float              f32;
typedef double             f64;
// Booleans
typedef u8                 b8;
typedef u32                b32;

#define false 0
#define true 1

typedef u64 IceDeviceSize;

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

//=================================================================================================
// Resource directories
//=================================================================================================
// TODO : Define these directories in a separate file
#define ICE_RESOURCE_TEXTURE_DIR "../../../Sandbox/res/textures/"
#define ICE_RESOURCE_SHADER_DIR "../../../Sandbox/res/shaders/compiled/"
#define ICE_RESOURCE_MODEL_DIR "../../../Sandbox/res/models/"

#endif // !DEFINES_H
