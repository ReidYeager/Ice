
#ifndef ICE_TOOLS_LOGGER_H_
#define ICE_TOOLS_LOGGER_H_

namespace Ice {

enum LogTypes
{
  Log_Type_Info,
  Log_Type_Debug,
  Log_Type_Warning,
  Log_Type_Error,
  Log_Type_Fatal
};

void LoggerAssembleMessage(LogTypes _type, const char* _message, ...);

}  // namespace Ice

#ifdef ICE_DEBUG
#define IceLogInfo(message, ...)                                        \
{                                                                       \
  Ice::LoggerAssembleMessage(Ice::Log_Type_Info, message, __VA_ARGS__); \
  Ice::LoggerAssembleMessage(Ice::Log_Type_Info, "\n");                 \
}

#define IceLogDebug(message, ...)                                        \
{                                                                        \
  Ice::LoggerAssembleMessage(Ice::Log_Type_Debug, message, __VA_ARGS__); \
  Ice::LoggerAssembleMessage(Ice::Log_Type_Debug, "\n");                 \
}

#define IceLogWarning(message, ...)                                        \
{                                                                          \
  Ice::LoggerAssembleMessage(Ice::Log_Type_Warning, message, __VA_ARGS__); \
  Ice::LoggerAssembleMessage(Ice::Log_Type_Warning, "\n");                 \
}
#else
#define IceLogInfo(message, ...)
#define IceLogDebug(message, ...)
#define IceLogWarning(message, ...)
#endif  // ICE_DEBUG

#define IceLogError(message, ...)                                        \
{                                                                        \
  Ice::LoggerAssembleMessage(Ice::Log_Type_Error, message, __VA_ARGS__); \
  Ice::LoggerAssembleMessage(Ice::Log_Type_Error, "\n");                 \
}

#define IceLogFatal(message, ...)                                        \
{                                                                        \
  Ice::LoggerAssembleMessage(Ice::Log_Type_Fatal, message, __VA_ARGS__); \
  Ice::LoggerAssembleMessage(Ice::Log_Type_Fatal, "\n");                 \
}

#endif  // ICE_TOOLS_LOGGER_H_
