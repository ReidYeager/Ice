
#ifndef ICE_PLATFORM_FILE_SYSTEM_H_
#define ICE_PLATFORM_FILE_SYSTEM_H_

#include "defines.h"

#include "rendering/mesh.h"

#include <vector>

class FileSystem
{
public:
  static std::vector<char> LoadFile(const char* _directory);
  // Loads a mesh with tinyobj and interprets it into an Ice mesh
  static mesh_t LoadMesh(const char* _directory);
  // Loads an image using stbi
  static void* LoadImageFile(const char* _directory, int& _width, int& _height);
  // Frees a(n) stbi image
  static void DestroyImageFile(void* _image);
};

#endif // !PLATFORM_FILE_SYSTEM_H
