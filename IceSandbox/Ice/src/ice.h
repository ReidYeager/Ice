
#ifndef ICE_H_
#define ICE_H_

#include "defines.h"

#include "core/application.h"

// Defined in the client application
extern b8 IceInitializeClient(IceClient* outApp);

int main()
{
  IceClient client;

  // Initialize the client information
  if (!IceInitializeClient(&client))
  {
    return -1;
  }

  // Ensure the client application defined its interface functions
  if (!client.Init || !client.Update || !client.Shutdown)
  {
    return -2;
  }

  // Initialize all required components
  if (!IceApplicationInitialize(&client))
  {
    // Ice failed to initialize a critical component
    return -3;
  }

  // Begin the main loop
  if (!IceApplicationLoop())
  {
    // Ice's main loop failed to complete
    return -4;
  }

  if (!IceApplicationShutdown())
  {
    // Ice failed to shutdown gracefully
    return -5;
  }

  return 0;
}

#endif // !ICE_H_
