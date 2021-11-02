
#ifndef ICE_RENDERER_SHADER_PARAMETERS_H_
#define ICE_RENDERER_SHADER_PARAMETERS_H_

#define ISBP(name, value, offset) Ice_Shader_Buffer_Param_##name = (u64)value << offset
// Represents information that the shaders may require as input
enum IceShaderBufferParameterFlagBits : IceFlagExtended
{
  // Make model matrix a guaranteed buffer input?
  // Would not require these definitions if it is guaranteed to be sent to the shader

  ISBP(ModelMatrix     , 15, 0 ), // 64 bytes of data
  ISBP(VpMatrix        , 15, 4 ), // 64 bytes of data
  ISBP(ViewMatrix      , 15, 8 ), // 64 bytes of data
  ISBP(ProjectionMatrix, 15, 12), // 64 bytes of data

  ISBP(User0, 1, 52), // 16 bytes of data
  ISBP(User1, 1, 53), // 16 bytes of data
  ISBP(User2, 1, 54), // 16 bytes of data
  ISBP(User3, 1, 55), // 16 bytes of data

  ISBP(User4, 1, 56), // 16 bytes of data
  ISBP(User5, 1, 57), // 16 bytes of data
  ISBP(User6, 1, 58), // 16 bytes of data
  ISBP(User7, 1, 59), // 16 bytes of data

  ISBP(User8,  1, 60), // 16 bytes of data
  ISBP(User9,  1, 61), // 16 bytes of data
  ISBP(User10, 1, 62), // 16 bytes of data
  ISBP(User11, 1, 63), // 16 bytes of data
};
typedef IceFlagExtended IceShaderBufferParameterFlags;
#undef ISP

#define ISIP(name, offset) Ice_Shader_Image_Param_##name = 1 << offset
enum IceShaderImageParameterFlagBits
{
  ISIP(DepthImage, 0),

  ISIP(User0, 22),
  ISIP(User1, 23),
  ISIP(User2, 24),
  ISIP(User3, 25),
  ISIP(User4, 26),
  ISIP(User5, 27),
  ISIP(User6, 28),
  ISIP(User7, 29),
  ISIP(User8, 30),
  ISIP(User9, 31)
};
typedef IceFlag IceShaderImageParameterFlags;
#undef ISIP

#endif // !define ICE_RENDERER_SHADER_PARAMETERS_H_
