
#ifndef ICE_SERVICE_HUB_H_
#define ICE_SERVICE_HUB_H_

#include "defines.h"

#include "platform/platform.h"
#include "renderer/renderer.h"

class ServiceHub
{
private:
  static inline IcePlatform* platform = nullptr;
  static inline IceRenderer* renderer = nullptr;

public:
  static void Initialize(IcePlatform* _platform = nullptr, IceRenderer* _renderer = nullptr)
  {
    if (_platform != nullptr)
      platform = _platform;
    if (_renderer != nullptr)
      renderer = _renderer;
  }

  static void Shutdown()
  {
    platform = nullptr;
    renderer = nullptr;
  }

  // Only used twice in vulkan_platform
  static IcePlatform* GetPlatform() { return platform; }
  // Completely unused
  static IceRenderer* GetRenderer() { return renderer; }
};

#endif // !DRYL_SERVICE_HUB_H_
