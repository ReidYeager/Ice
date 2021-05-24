
#ifndef DEFINES_H
#define DEFINES_H 1

#ifdef ICE_DLL_EXPORT
#define ICE_API __declspec(dllexport)
#else
#define ICE_API __declspec(dllimport)
#endif // ICE_DLL_EXPORT

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
#error "Only windows supported"
#endif // defined(WIN32) || defined(_WIN32) || defined(__WIN32__)

//=================================================================================================
// Ice Flags
//=================================================================================================
typedef u32 IceFlag;

//=================================================================================================
// Ice Error Codes
//=================================================================================================
enum IceErrorCodeBits
{
  ICE_EC_NONE,
  ICE_EC_FAILURE,
  ICE_EC_CATASTROPHIC,
  ICE_EC_COUNT
};
typedef IceFlag iceErrorCodeFlag;


#endif // !DEFINES_H
