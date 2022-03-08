
#ifndef ICE_ZCORE_ZAPPLICATION_H_
#define ICE_ZCORE_ZAPPLICATION_H_

#include "defines.h"

#include "zplatform/zplatform.h"
#include "rendering/renderer.h"

#include <stdio.h>

extern IceCamera cam;

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

  IceObject* zAddObject(const char* _meshDir, u32 _material, IceObject* _parent = nullptr);
  // Creates a new material instance using the input shaders for subpass 0
  u32 zCreateMaterial(IceMaterialTypes _type, std::vector<IceShader> _shaders);
  // Creates a new material for subpass 1
  u32 zCreateLightingMaterial(std::vector<IceShader> _shaders);
  b8 zSetLightingMaterial(IceHandle _material);
  // Updates the material texture samplers
  void zAssignMaterialTextures(IceHandle _material, std::vector<IceTexture> _textures);
  b8 zSetMaterialBufferData(IceHandle _material, void* _data);
}

#endif // !ICE_ZCORE_ZAPPLICATION_H_
