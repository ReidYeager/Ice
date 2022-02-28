
#ifndef ICE_LOGGER_H_
#define ICE_LOGGER_H_

#include "defines.h"
#include "zplatform/zplatform.h"

#include <stdarg.h>
#include <stdio.h>

enum IceLogTypes
{
  Ice_Log_Info,
  Ice_Log_Debug,
  Ice_Log_Warning,
  Ice_Log_Error,
  Ice_Log_Fatal
};

inline void ICE_API IceConsoleLogMessage(IceLogTypes _type, const char* _message, ...)
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

#ifdef ICE_DEBUG
#define IceLogInfo(message, ...)                            \
{                                                           \
  IceConsoleLogMessage(Ice_Log_Info, message, __VA_ARGS__); \
  IceConsoleLogMessage(Ice_Log_Info, "\n"); \
}

#define IceLogDebug(message, ...)                            \
{                                                            \
  IceConsoleLogMessage(Ice_Log_Debug, message, __VA_ARGS__); \
  IceConsoleLogMessage(Ice_Log_Debug, "\n");                 \
}

#define IceLogWarning(message, ...)                            \
{                                                              \
  IceConsoleLogMessage(Ice_Log_Warning, message, __VA_ARGS__); \
  IceConsoleLogMessage(Ice_Log_Warning, "\n");                 \
}
#else
#define IceLogInfo(message, ...)
#define IceLogDebug(message, ...)
#define IceLogWarning(message, ...)
#endif // ICE_DEBUG

#define IceLogError(message, ...)                            \
{                                                            \
  IceConsoleLogMessage(Ice_Log_Error, message, __VA_ARGS__); \
  IceConsoleLogMessage(Ice_Log_Error, "\n");                 \
}

#define IceLogFatal(message, ...)                            \
{                                                            \
  IceConsoleLogMessage(Ice_Log_Fatal, message, __VA_ARGS__); \
  IceConsoleLogMessage(Ice_Log_Fatal, "\n");                 \
}

#endif // !DRYL_LOGGER_H

