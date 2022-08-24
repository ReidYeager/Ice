
#ifndef ICE_CORE_APPLICATION_H_
#define ICE_CORE_APPLICATION_H_

#include "defines.h"
#include "core/application_defines.h"

#include "platform/platform.h"
#include "rendering/renderer.h"
#include "core/ecs/ecs.h"

namespace Ice {

  //=========================
  // Application
  //=========================

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

  b8 GetMesh(const char* _directory, Ice::Mesh** _mesh);

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

  /*
  Ice::Scene* CreateScene(u32 _maxObjectCount = 100, u32 _maxComponentTypeCount = 10);
  void DestroyScene(Ice::Scene* _scene);
  // Used to create objects in the active scene without needing to pass its pointer around the game

  void SetActiveScene(Ice::Scene* _scene);
  Ice::Scene* GetActiveScene();

  void AddSceneToRender(Ice::Scene* _scene);
  */

  Ice::Entity CreateCamera(Ice::Scene* _scene, Ice::CameraSettings _settings = {});
  Ice::Entity CreateRenderedEntity(Ice::Scene* _scene,
                                   const char* _meshDir = nullptr,
                                   Ice::Material* _material = nullptr);

  b8 UpdateTransforms();

  void TMPSetMainScene(Ice::Scene* _scene);

} // namespace Ice

#endif // !ICE_CORE_APPLICATION_H_
