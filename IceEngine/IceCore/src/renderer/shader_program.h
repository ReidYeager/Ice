
#ifndef RENDERER_SHADER_PROGRAM_H
#define RENDERER_SHADER_PROGRAM_H 1

#include "defines.h"
#include <vector>

// NOTE : Move to defines?
enum IceShaderStageFlagBits
{
  ICE_SHADER_STAGE_VERT = 0x01, // vertex shader
  ICE_SHADER_STAGE_FRAG = 0x02, // fragment shader
  ICE_SHADER_STAGE_COMP = 0x04  // compute shader
};
typedef IceFlag IceShaderStageFlags;

#ifdef ICE_VULKAN
#include <vulkan/vulkan.h>
struct iceShader_t
{
  const char* name;
  IceShaderStageFlags stage;
  VkShaderModule module;
  // shader bindings
};
#else
struct iceShader_t
{
  const char* name;
  IceShaderStageFlags stage;
};
#endif // ICE_VULKAN

struct iceShaderProgram_t
{
  const char* name;
  IceShaderStageFlags stages;

  u16 vertIndex;
  u16 fragIndex;
  u16 compIndex;
};

#endif // !RENDERER_SHADER_PROGRAM_H
