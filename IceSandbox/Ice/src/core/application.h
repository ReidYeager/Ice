
#ifndef ICE_CORE_RE_APPLICATION_H_
#define ICE_CORE_RE_APPLICATION_H_

#include "defines.h"

#include "platform/platform.h"
#include "rendering/renderer.h"

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

struct reIceApplicationSettings
{
  const char* title;
  u32 version;

  reIceWindowSettings windowSettings;
  reIceRendererSettings rendererSettings;

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

  u32 Run(reIceApplicationSettings* _settings);

  void AddObject(const char* _meshDir, u32 _material);
  u32 CreateMaterial(std::vector<IceShaderInfo> _shaders);

private:
  b8 Initialize(reIceApplicationSettings* _settings);
  b8 Update();
  b8 Shutdown();

};

#endif // !define ICE_CORE_RE_APPLICATION_H_
