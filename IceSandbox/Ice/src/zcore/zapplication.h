
#ifndef ICE_ZCORE_ZAPPLICATION_H_
#define ICE_ZCORE_ZAPPLICATION_H_

#include "defines.h"

#include "zplatform/zplatform.h"
#include "rendering/renderer.h"

#include <stdio.h>

namespace Ice
{
  struct zIceApplicationSettings
  {
    zIceWindowSettings windowSettings;

    IceRendererSettings rendererSettings;

    b8(*gameInit)() = 0;
    b8(*gameUpdate)() = 0;
    b8(*gameShutdown)() = 0;
  };

  b8 Run(zIceApplicationSettings _settings);
  
}

#endif // !ICE_ZCORE_ZAPPLICATION_H_
