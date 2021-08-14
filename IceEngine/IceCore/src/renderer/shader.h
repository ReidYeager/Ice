
#ifndef ICE_RENDERER_SHADER_H_
#define ICE_RENDERER_SHADER_H_

#include "defines.h"
#include "renderer/vulkan/vulkan_buffer.h"
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
// Each enum represents 16 bytes
enum IceShaderBufferParameterFlagBits : IceFlagExtended
{
  ISP(Mvp_X) = (u64)1 << 0, // X vector of MVP matrix
  ISP(Mvp_Y) = (u64)1 << 1, // Y vector of MVP matrix
  ISP(Mvp_Z) = (u64)1 << 2, // Z vector of MVP matrix
  ISP(Mvp_W) = (u64)1 << 3, // W vector of MVP matrix

  ISP(Model_X) = (u64)1 << 4, // X vector of model matrix
  ISP(Model_Y) = (u64)1 << 5, // Y vector of model matrix
  ISP(Model_Z) = (u64)1 << 6, // Z vector of model matrix
  ISP(Model_W) = (u64)1 << 7, // W vector of model matrix

  ISP(View_X) = (u64)1 << 8,  // X vector of view matrix
  ISP(View_Y) = (u64)1 << 9,  // Y vector of view matrix
  ISP(View_Z) = (u64)1 << 10, // Z vector of view matrix
  ISP(View_W) = (u64)1 << 11, // W vector of view matrix

  ISP(Projection_X) = (u64)1 << 12, // X vector of projection matrix
  ISP(Projection_Y) = (u64)1 << 13, // Y vector of projection matrix
  ISP(Projection_Z) = (u64)1 << 14, // Z vector of projection matrix
  ISP(Projection_W) = (u64)1 << 15, // W vector of projection matrix

  ISP(User0) = (u64)1 << 53,
  ISP(User1) = (u64)1 << 54,
  ISP(User2) = (u64)1 << 55,
  ISP(User3) = (u64)1 << 56,
  ISP(User4) = (u64)1 << 57,
  ISP(User5) = (u64)1 << 58,
  ISP(User6) = (u64)1 << 59,
  ISP(User7) = (u64)1 << 60,
  ISP(User8) = (u64)1 << 61,
  ISP(User9) = (u64)1 << 62,

  ISP(Max) = (u64)1 << 63
};
typedef IceFlagExtended IceShaderBufferParameterFlags;
#undef ISP

struct IceShaderInfo
{
  // TODO : Find a better way of identifying what shaders an IvkShader uses
  std::vector<const char*> sources;
  IceShaderStageFlags stages;
  std::vector<IceShaderBindingFlags> bindings;
  // TODO : Replace with API agnostic buffer
  IvkBuffer* buffer;
  IceShaderBufferParameterFlags bufferContentFlags;
  // TODO : Replace with indices for a texture flyweight
  std::vector<const char*> textures;
};

#endif // !ICE_RENDERER_SHADER_H_
