
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
    u32 version = 0;

    b8(*clientInitFunction)();
    b8(*clientUpdateFunction)(f32 _delta);
    b8(*clientShutdownFunction)();

    Ice::RendererSettings renderer;
    Ice::WindowSettings window;

    // I don't really like this.
    u32 maxShaderCount = 200;   // Number of unique shaders
    u32 maxMaterialCount = 100; // Number of unique materials
    u32 maxMeshCount = 200;     // Number of unique meshes
    u32 maxObjectCount = 200;   // Number of unique objects
    u32 maxTextureCount = 100;  // Number of unique textures
  };

  u32 CreateVersion(u8 _major, u8 _minor, u8 _patch);

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

  b8 CreateMesh(const char* _directory, Ice::Mesh** _mesh);

  b8 CreateMaterial(Ice::MaterialSettings _settings, Ice::Material** _material = nullptr);
  void SetMaterialData(Ice::Material* _material, Ice::BufferSegment _segment, void* _data);

  // Reload one shader
  b8 ReloadShader(Ice::Shader* _shader);
  b8 ReloadAllShaders();

  // Reload material's shaders and recreate the material
  b8 ReloadMaterial(Ice::Material* _material);
  b8 ReloadAllMaterials();
  // Only recreate the material -- Does not affect its shaders
  b8 RecreateMaterial(Ice::Material* _material);

  b8 LoadTexture(Ice::Image* _texture, const char* _directory);
  void SetTexture(Ice::Material* _material, u32 _inputIndex, const char* _image);
  //void SetTexture(Ice::Material* _material, u32 inputIndex, Ice::Image* _image);

  //=========================
  // Scenes
  //=========================

  Ice::Scene* CreateScene(u32 _maxObjectCount = 100, u32 _maxComponentTypeCount = 10);
  void DestroyScene(Ice::Scene* _scene);
  // Used to create objects in the active scene without needing to pass its pointer around the game

  void SetActiveScene(Ice::Scene* _scene);
  Ice::Scene* GetActiveScene();

  Ice::Object* CreateObject(const char* _meshDir, Ice::Material* _material);
  Ice::Object* CreateCamera(Ice::CameraSettings _settings = {});

  void AddSceneToRender(Ice::Scene* _scene);
  void UpdateTransforms();

}
#endif // !ICE_CORE_APPLICATION_H_
