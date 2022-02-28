
#include "logger.h"

#include "platform/file_system.h"
#include "platform/platform.h"
#include "rendering/mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <fstream>
#include <string>
#include <unordered_map>

std::vector<char> Ice::LoadFile(const char* _directory)
{
  std::ifstream inFile;
  inFile.open(_directory, std::ios::ate | std::ios::binary);
  if (!inFile)
  {
    IceLogWarning("Failed to load file : %s", _directory);
    return {};
  }

  size_t fileSize = inFile.tellg();
  inFile.seekg(0);
  std::vector<char> rawData(fileSize);
  inFile.read(rawData.data(), fileSize);

  inFile.close();
  return rawData;
}

IvkMesh Ice::LoadMesh(const char* _directory)
{
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string loadWarnings, loadErrors;

  std::string dir(ICE_RESOURCE_MODEL_DIR);
  dir.append(_directory);

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &loadWarnings, &loadErrors, dir.c_str()))
  {
    IceLogFatal("Failed to load mesh %s\n\ttinyobj warnings: %s\n\ttinyobj erros: %s",
                _directory, loadWarnings.c_str(), loadErrors.c_str());
    return {};
  }

  IvkMesh mesh;
  std::unordered_map<iceVertex, u32> vertMap = {};

  iceVertex vert {};
  for (const auto& shape : shapes)
  {
    for (const auto& index : shape.mesh.indices)
    {
      vert.position = {
        attrib.vertices[3 * index.vertex_index + 0],
        attrib.vertices[3 * index.vertex_index + 1],
        attrib.vertices[3 * index.vertex_index + 2]
      };

      vert.uv = {
        attrib.texcoords[2 * index.texcoord_index + 0],
        1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
      };

      vert.normal = {
        attrib.normals[3 * index.normal_index + 0],
        attrib.normals[3 * index.normal_index + 1],
        attrib.normals[3 * index.normal_index + 2]
      };

      if (vertMap.count(vert) == 0)
      {
        vertMap[vert] = (u32)mesh.vertices.size();
        mesh.vertices.push_back(vert);
      }
      mesh.indices.push_back(vertMap[vert]);
    }
  }

  // Create vertex buffer
  // Create index buffer

  std::string fileDir(_directory);
  mesh.directory = fileDir.c_str();

  IceLogInfo("Loaded %s -- %u verts, %u indices",
             _directory, mesh.vertices.size(),
             mesh.indices.size());

  return mesh;
}

#include <stdio.h>
#include <direct.h>
#include <limits.h>
#include <iostream>

void* Ice::LoadImageFile(const char* _directory, int& _width, int& _height)
{
  std::string dir(ICE_RESOURCE_TEXTURE_DIR);
  dir.append(_directory);

  IceLogInfo("Attempting to load image '%s'", dir.c_str());

  int channels;
  stbi_uc* image = stbi_load(dir.c_str(), &_width, &_height, &channels, STBI_rgb_alpha);
  assert (image != nullptr);
  return image;
}

void Ice::DestroyImageFile(void* _image)
{
  stbi_image_free(_image);
}

