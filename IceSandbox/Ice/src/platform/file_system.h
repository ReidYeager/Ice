
#ifndef ICE_PLATFORM_FILE_SYSTEM_H_
#define ICE_PLATFORM_FILE_SYSTEM_H_

#include "defines.h"

#include "rendering/mesh.h"

#include <vector>

extern class IceFileSystem
{
public:
  // Loads the given file as binary
  std::vector<char> LoadFile(const char* _directory);
  // Loads a mesh with tinyobj and interprets it into an Ice mesh
  IvkMesh LoadMesh(const char* _directory);
  // Loads an image using stbi
  void* LoadImageFile(const char* _directory, int& _width, int& _height);
  // Frees a stbi image
  void DestroyImageFile(void* _image);
} fileSystem;

#endif // !PLATFORM_FILE_SYSTEM_H
