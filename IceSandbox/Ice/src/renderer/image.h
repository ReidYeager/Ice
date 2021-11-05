
#ifndef ICE_RENDERER_IMAGE_H_
#define ICE_RENDERER_IMAGE_H_

#include "defines.h"

#include <string>

//struct IceImage
//{
//  
//};

struct iceTexture_t
{
  u32 imageIndex;
  // const char* directory is lost when texture vector expands, using a string fixes this
  std::string directory;

  iceTexture_t(std::string _dir) : imageIndex(0), directory(_dir) {}
};

#endif // !define ICE_CORE_IMAGE_H_
