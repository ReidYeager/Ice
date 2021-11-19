
#ifndef ICE_CORE_RE_APPLICATION_H_
#define ICE_CORE_RE_APPLICATION_H_

#include "defines.h"

#include "platform/re_platform.h"

struct reIceApplicationSettings
{
  reIceWindowSettings windowSettings;

  void(*ClientInitialize)();
  void(*ClientUpdate)(float);
  void(*ClientShutdown)();
};

class reIceApplication
{
private:
  // Platform interface
  // Render interface

  struct
  {
    void(*ClientInitialize)();
    void(*ClientUpdate)(float);
    void(*ClientShutdown)();
  } state;

public:
  void Run(reIceApplicationSettings* _settings);

private:
  b8 Initialize(reIceApplicationSettings* _settings);
  b8 Update();
  b8 Shutdown();

};

#endif // !define ICE_CORE_RE_APPLICATION_H_
