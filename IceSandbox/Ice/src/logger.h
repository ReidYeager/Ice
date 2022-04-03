
#ifndef ICE_LOGGER_H_
#define ICE_LOGGER_H_

#include "defines.h"

#include "platform/platform.h"

#include <stdarg.h>
#include <stdio.h>

namespace Ice {
  enum LogTypes
  {
    Log_Info,
    Log_Debug,
    Log_Warning,
    Log_Error,
    Log_Fatal
  };

  inline void ICE_API ConsoleLogMessage(Ice::LogTypes _type, const char* _message, ...)
  {
    // Limit 65,535 characters per message
    const u16 length = 0xFFFF;
    char* outMessage = new char[length];
    //rePlatform.MemZero(outMessage, length);
    Ice::MemoryZero(outMessage, length);

    va_list args;
    va_start(args, _message);
    vsnprintf(outMessage, length, _message, args);
    va_end(args);

    //rePlatform.ConsolePrint(outMessage, _type);
    Ice::PrintToConsole(outMessage, _type);

    delete[](outMessage);
  }
}


#ifdef ICE_DEBUG
#define IceLogInfo(message, ...)                               \
{                                                              \
  Ice::ConsoleLogMessage(Ice::Log_Info, message, __VA_ARGS__); \
  Ice::ConsoleLogMessage(Ice::Log_Info, "\n"); \
}

#define IceLogDebug(message, ...)                               \
{                                                               \
  Ice::ConsoleLogMessage(Ice::Log_Debug, message, __VA_ARGS__); \
  Ice::ConsoleLogMessage(Ice::Log_Debug, "\n");                 \
}

#define IceLogWarning(message, ...)                               \
{                                                                 \
  Ice::ConsoleLogMessage(Ice::Log_Warning, message, __VA_ARGS__); \
  Ice::ConsoleLogMessage(Ice::Log_Warning, "\n");                 \
}
#else
#define IceLogInfo(message, ...)
#define IceLogDebug(message, ...)
#define IceLogWarning(message, ...)
#endif // ICE_DEBUG

#define IceLogError(message, ...)                               \
{                                                               \
  Ice::ConsoleLogMessage(Ice::Log_Error, message, __VA_ARGS__); \
  Ice::ConsoleLogMessage(Ice::Log_Error, "\n");                 \
}

#define IceLogFatal(message, ...)                               \
{                                                               \
  Ice::ConsoleLogMessage(Ice::Log_Fatal, message, __VA_ARGS__); \
  Ice::ConsoleLogMessage(Ice::Log_Fatal, "\n");                 \
}

#endif // !DRYL_LOGGER_H

