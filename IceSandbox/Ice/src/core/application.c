
#include "defines.h"

#include "core/application.h"
#include "platform/platform.h"
#include "math/vectors.h"

#include <stdio.h>
#include <stdlib.h>

IceEngineState* engine;

b8 IceApplicationInitialize(IceClient* _client)
{
  if (engine)
  {
    printf("App already defined!\n");
    return false;
  }

  // TODO : ~!!~ Memory management
  engine = malloc(sizeof(IceEngineState));
  if (!engine)
  {
    printf("Somehow failed to allocate the engine state\n");
    return false;
  }
  engine->client = _client;

  // Initialize the platform
  {
    vec2I windowPos = { 50, 50 };
    vec2U windowExtents = { 800, 600 };
    IcePlatformCreateWindow(&(engine->platformState),
                            windowPos,
                            windowExtents,
                            "Ice-C");
  }

  engine->inputState = 0;
  engine->rendererState = 0;

  // Initialize the client
  _client->engineState = engine;
  _client->Init(_client);

  engine->isRunning = true;
  return true;
}

b8 IceApplicationLoop()
{
  // TODO : time & deltatime

  while (IcePlatformUpdate(engine->platformState))
  {
    engine->client->Update(engine->client, 0.0f);
  }

  engine->isRunning = false;
  return true;
}

ICE_API b8 IceApplicationShutdown()
{
  engine->client->Shutdown(engine->client);

  // Shutdown engine components
  IcePlatformShutdown(engine->platformState);

  engine->client->engineState = 0;
  free(engine);
  return true;
}

