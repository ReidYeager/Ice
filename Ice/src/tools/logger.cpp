
#include "defines.h"

#include "tools/logger.h"
#include "core/platform/platform.h"

// Not in logger.h to use engine platform functions
//   instead of having some platform specific functionality separate from the rest
void Ice::LoggerAssembleMessage(Ice::LogTypes _type, const char* _message, ...)
{
  // Limit 2048 characters per message
  const u16 length = 0x800;
  char* outMessage = (char*)Ice::MemoryAllocZero(length);

  va_list args;
  va_start(args, _message);
  vsnprintf(outMessage, length, _message, args);
  va_end(args);

  Ice::PrintToConsole(outMessage, _type);

  Ice::MemoryFree(outMessage);
}
