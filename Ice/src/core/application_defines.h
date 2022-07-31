
#ifndef ICE_CORE_APPLICATION_DEFINES_H_
#define ICE_CORE_APPLICATION_DEFINES_H_

#include "defines.h"

#include "math/matrix.hpp"
#include "platform/platform_defines.h"
#include "rendering/renderer_defines.h"

namespace Ice {

//=========================
// Application
//=========================

struct ApplicationSettings
{
  u32 version = 0;

  b8(*GameInit)();
  b8(*GameUpdate)(f32 _delta);
  b8(*GameShutdown)();

  Ice::RendererSettingsCore rendererCore;
  Ice::WindowSettings window;

  // I don't really like this.
  u32 maxShaderCount = 200;   // Number of unique shaders
  u32 maxMaterialCount = 100; // Number of unique materials
  u32 maxMeshCount = 200;     // Number of unique meshes
  u32 maxObjectCount = 200;   // Number of unique objects
  u32 maxTextureCount = 100;  // Number of unique textures
};

} // namespace Ice

#endif // !ICE_CORE_APPLICATION_DEFINES_H_
