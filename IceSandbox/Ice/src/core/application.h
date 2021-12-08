
#ifndef ICE_CORE_RE_APPLICATION_H_
#define ICE_CORE_RE_APPLICATION_H_

#include "defines.h"

#include "platform/platform.h"
#include "rendering/renderer.h"

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
  u32 Run(reIceApplicationSettings* _settings);

private:
  b8 Initialize(reIceApplicationSettings* _settings);
  b8 Update();
  b8 Shutdown();

};

#endif // !define ICE_CORE_RE_APPLICATION_H_
