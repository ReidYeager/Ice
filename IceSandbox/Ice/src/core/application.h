
#ifndef ICE_CORE_APPLICATION_H_
#define ICE_CORE_APPLICATION_H_

#include "defines.h"

#include "platform/platform.h"
#include "rendering/renderer.h"

namespace Ice {

  //=========================
  // Time
  //=========================
  extern struct IceTime
  {
    // Real-time in seconds since the application started
    union
    {
      f32 totalTime;
      f32 realTime;
    };

    f32 deltaTime; // Time in seconds taken by the previous tick

    u32 frameCount; // Number of frames rendered before this tick
  } time;

  //=========================
  // Application
  //=========================

  struct ApplicationSettings
  {
    b8(*clientInitFunction)();
    b8(*clientUpdateFunction)(f32 _delta);
    b8(*clientShutdownFunction)();

    Ice::RendererSettings renderer;
    Ice::WindowSettings window;

    u32 maxShaderCount = 200;
    u32 maxMaterialCount = 100;
  };

  u32 Run(ApplicationSettings);

  // End the application loop peacefully
  void Shutdown();

  //=========================
  // Platform
  //=========================

  void CloseWindow();

  //=========================
  // Rendering
  //=========================

  Ice::Material& CreateMaterial(Ice::MaterialSettings _settings);

}
#endif // !ICE_CORE_APPLICATION_H_
