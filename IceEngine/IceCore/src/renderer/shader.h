
#ifndef ICE_RENDERER_SHADER_H_
#define ICE_RENDERER_SHADER_H_

#include "defines.h"

#include <vector>

enum IceShaderStageFlagBits
{
  Ice_Shader_Vert = 0x01, // vertex shader
  Ice_Shader_Frag = 0x02, // fragment shader
  Ice_Shader_Comp = 0x04  // compute shader
};
typedef IceFlag IceShaderStageFlags;

enum IceShaderBindingFlagBits
{
  Ice_Shader_Binding_Buffer = 0x00,
  Ice_Shader_Binding_Image = 0x01,
};
typedef IceFlag IceShaderBindingFlags;

#define ISP(name) Ice_Shader_Param_##name
// TODO : Pre-define information that the shaders might need
// Each enum represents 4 bytes
enum IceShaderParameters
{
  ISP(Mvp_X) = 0,

  ISP(User0),
  ISP(User1),

  ISP(Max)
};
#undef ISP

struct IvkShaderBufferInfo
{
  u32 size;
  u32 offset;
  std::vector<IceShaderParameters> dataFlags;
};

struct IceShaderInfo
{
  // TODO : Find a better way of identifying what shaders an IvkShader uses
  std::vector<const char*> sources;
  IceShaderStageFlags stages;
  IvkShaderBufferInfo bufferInfo;
  // TODO : Replace with indices for a texture flyweight
  std::vector<const char*> textures;
};

#endif // !ICE_RENDERER_SHADER_H_
