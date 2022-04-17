
#ifndef ICE_CORE_APPLICATION_H_
#define ICE_CORE_APPLICATION_H_

#include "defines.h"

#include "platform/platform.h"
#include "rendering/renderer.h"
#include "core/ecs.h"
#include "core/scene.h"

namespace Ice {

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
    u32 maxObjectCount = 200;   // Number of unique objects
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
  Ice::Object& CreateObject();
  void AttatchRenderComponent(Ice::Object* _object, const char* _meshDir, Ice::Material* _material);
  void AttatchCameraComponent(Ice::Object* _object, Ice::CameraSettings _settings);

  void UpdateTransforms();

}
#endif // !ICE_CORE_APPLICATION_H_
