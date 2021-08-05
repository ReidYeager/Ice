
#ifndef ICE_ASSERTS_H_
#define ICE_ASSERTS_H_

// Comment this out to disable asserts
#define ENABLE_ICE_ASSERTS

#ifdef ENABLE_ICE_ASSERTS

#include "logger.h"
#include <intrin.h>

#define ICE_ASSERT(expression)                                                 \
{                                                                           \
  if (!(expression))                                                        \
  {                                                                         \
    LogFatal("%s failed -- Line %d : %s", #expression, __LINE__, __FILE__); \
    __debugbreak();                                                         \
  }                                                                         \
}

#define ICE_ASSERT_MSG(expression, msg, ...)                                   \
{                                                                           \
  if (!(expression))                                                        \
  {                                                                         \
    LogFatal("%s failed -- Line %d : %s", #expression, __LINE__, __FILE__); \
    LogFatal(msg, __VA_ARGS__);                                             \
    __debugbreak();                                                         \
  }                                                                         \
}

#else
#define ICE_ASSERT(expression)
#endif // ENABLE_DASSERTS

#endif // !DRYL_ASSERTS_H_
