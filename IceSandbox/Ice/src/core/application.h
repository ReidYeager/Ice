
#ifndef ICE_CORE_RE_APPLICATION_H_
#define ICE_CORE_RE_APPLICATION_H_

#include "defines.h"

#include "core/scene.h"
#include "platform/platform.h"
#include "rendering/renderer.h"

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

struct IceApplicationSettings
{
  const char* title;
  u32 version;

  IceWindowSettings windowSettings;
  IceRendererSettings rendererSettings;

  void(*ClientInitialize)();
  void(*ClientUpdate)(float);
  void(*ClientShutdown)();
};

class reIceApplication
{
private:
  struct
  {
    void(*ClientInitialize)();
    void(*ClientUpdate)(float);
    void(*ClientShutdown)();
  } state;

public:
  IceCamera cam;
  IceObject* sceneRoot;

  u32 Run(IceApplicationSettings* _settings);

  IceObject* AddObject(const char* _meshDir, u32 _material, IceObject* _parent = nullptr);
  // Creates a new material instance using the input shaders
  u32 CreateMaterial(std::vector<IceShader> _shaders);
  // Updates the material texture samplers
  void AssignMaterialTextures(IceHandle _material, std::vector<std::string> _images);

private:
  b8 Initialize(IceApplicationSettings* _settings);
  b8 Update();
  b8 Shutdown();

};

#endif // !define ICE_CORE_RE_APPLICATION_H_
