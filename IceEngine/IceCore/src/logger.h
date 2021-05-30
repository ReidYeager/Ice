
#ifndef LOGGER_H
#define LOGGER_H 1

#include "platform/platform.h"

#define IPrint(message, ...)                        \
{                                                   \
  Platform::PrintToConsole(message, __VA_ARGS__);   \
  Platform::PrintToConsole("\n");   \
}

#endif // !LOGGER_H
