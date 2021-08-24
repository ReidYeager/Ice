
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
  // TODO : Fix const char* directory being lost when texture vector expands
  std::string directory;

  iceTexture_t(std::string _dir) : imageIndex(0), directory(_dir) {}
};

#endif // !define ICE_CORE_IMAGE_H_
