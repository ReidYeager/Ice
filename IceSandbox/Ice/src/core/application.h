
#ifndef ICE_CORE_APPLICATION_H_
#define ICE_CORE_APPLICATION_H_

#include "defines.h"

// The engine's interface with the client application
typedef struct IceClient
{
  void(*Init)(struct IceClient* _app);
  void(*Update)(struct IceClient* _app, f32 _deltaTime);
  void(*Shutdown)(struct IceClient* _app);
  //void(*GameResized)(struct IceApplication* _app, u32 _width, u32 _height);

  // information about the engine's internal state
  void* engineState;
} IceClient;

typedef struct IceEngineState
{
  IceClient* client;

  b8 isRunning;

  void* platformState;
  void* inputState;
  void* rendererState;
} IceEngineState;

// Initialize the engine and its components for rendering
// Returns : true if all components are created successfully, false otherwise
// Parameter : _client : The client application's state information
ICE_API b8 IceApplicationInitialize(IceClient* _app);
// Renders each frame until the application is closed
// Returns : true
ICE_API b8 IceApplicationLoop();
// Destroys and frees the engine and all of its components
// Returns : true
ICE_API b8 IceApplicationShutdown();

#endif // !define ICE_CORE_APPLICATION_H_
