
#ifndef ICE_PLATFORM_FILE_SYSTEM_H_
#define ICE_PLATFORM_FILE_SYSTEM_H_

#include "defines.h"
#include "renderer/mesh.h"

#include <vector>


class FileSystem
{
public:
  static std::vector<char> LoadFile(const char* _directory);
  static mesh_t LoadMesh(const char* _directory);
  static void* LoadImageFile(const char* _directory, int& _width, int& _height);
  static void DestroyImageFile(void* _image);
};

#endif // !PLATFORM_FILE_SYSTEM_H
