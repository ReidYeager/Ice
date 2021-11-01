
#ifndef ICE_RENDERER_SHADER_PARAMETERS_H_
#define ICE_RENDERER_SHADER_PARAMETERS_H_

#define ISP(name, value, offset) Ice_Shader_Param_##name = (u64)value << offset
// Represents information that the shaders may require as input
enum IceShaderBufferParameterFlagBits : IceFlagExtended
{
  // Make model matrix a guaranteed buffer input?
  // Would not require these definitions if it is guaranteed to be sent to the shader

  ISP(ModelMatrix     , 15, 0 ), // 64 bytes of data
  ISP(VpMatrix        , 15, 4 ), // 64 bytes of data
  ISP(ViewMatrix      , 15, 8 ), // 64 bytes of data
  ISP(ProjectionMatrix, 15, 12), // 64 bytes of data

  ISP(User0, 1, 52), // 16 bytes of data
  ISP(User1, 1, 53), // 16 bytes of data
  ISP(User2, 1, 54), // 16 bytes of data
  ISP(User3, 1, 55), // 16 bytes of data

  ISP(User4, 1, 56), // 16 bytes of data
  ISP(User5, 1, 57), // 16 bytes of data
  ISP(User6, 1, 58), // 16 bytes of data
  ISP(User7, 1, 59), // 16 bytes of data

  ISP(User8,  1, 60), // 16 bytes of data
  ISP(User9,  1, 61), // 16 bytes of data
  ISP(User10, 1, 62), // 16 bytes of data
  ISP(User11, 1, 63), // 16 bytes of data
};
typedef IceFlagExtended IceShaderBufferParameterFlags;
#undef ISP

#endif // !define ICE_RENDERER_SHADER_PARAMETERS_H_
