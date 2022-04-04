
#ifndef ICE_CORE_APPLICATION_H_
#define ICE_CORE_APPLICATION_H_

#include "defines.h"

#include "platform/platform.h"
#include "rendering/renderer.h"
#include "core/ecs.h"

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

    u32 maxShaderCount = 200;   // Number of unique shaders
    u32 maxMaterialCount = 100; // Number of unique materials
    u32 maxMeshCount = 200;     // Number of unique meshes
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

  b8 CreateMaterial(Ice::MaterialSettings _settings, Ice::Material** _material = nullptr);
  Ice::Entity CreateObject(const char* _meshDir, Ice::Material* _material);

}
#endif // !ICE_CORE_APPLICATION_H_
