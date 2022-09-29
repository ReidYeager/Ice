
#include "defines.h"

#include "core/platform/platform.h"

void Ice::ConsoleLogMessage(Ice::LogTypes _type, const char* _message, ...)
{
  // Limit 2048 characters per message
  const u16 length = 0x800;
  char* outMessage = (char*)Ice::MemoryAllocZero(length);
  //char* outMessage = new char[length];
  //Ice::MemoryZero(outMessage, length);

  va_list args;
  va_start(args, _message);
  vsnprintf(outMessage, length, _message, args);
  va_end(args);

  Ice::PrintToConsole(outMessage, _type);

  Ice::MemoryFree(outMessage);
  //delete[](outMessage);
}
