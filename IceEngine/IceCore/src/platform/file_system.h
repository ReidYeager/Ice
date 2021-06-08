
#ifndef PLATFORM_FILE_SYSTEM_H
#define PLATFORM_FILE_SYSTEM_H 1

#include "defines.h"
#include "renderer/mesh.h"

#include <vector>


class FileSystem
{
public:
  static std::vector<char> LoadFile(const char* _directory);
  static mesh_t LoadMesh(const char* _directory);
};

#endif // !PLATFORM_FILE_SYSTEM_H
