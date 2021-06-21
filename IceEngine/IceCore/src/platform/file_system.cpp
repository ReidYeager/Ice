
#include <fstream>
#include <string>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "logger.h"
#include "platform/file_system.h"
#include "platform/platform.h"
#include "renderer/mesh.h"

std::vector<char> FileSystem::LoadFile(const char* _directory)
{
  std::ifstream inFile;
  inFile.open(_directory, std::ios::ate | std::ios::binary);
  if (!inFile)
  {
    // TODO : Log non-Vulkan errors
    IcePrint("ERROR :: Failed to load file %s", _directory);
    return {};
  }

  size_t fileSize = inFile.tellg();
  inFile.seekg(0);
  std::vector<char> rawData(fileSize);
  inFile.read(rawData.data(), fileSize);

  inFile.close();
  return rawData;
}

mesh_t FileSystem::LoadMesh(const char* _directory)
{
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string loadWarnings, loadErrors;

  std::string dir("res/models/");
  dir.append(_directory);

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &loadWarnings, &loadErrors, dir.c_str()))
  {
    IcePrint("Failed to load object\n\tWarnings: %s\n\tErros: %s",
             loadWarnings.c_str(), loadErrors.c_str());
    return {};
  }

  mesh_t mesh;
  std::unordered_map<vertex_t, u32> vertMap = {};

  vertex_t vert {};
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

  IcePrint("Loaded %s -- %u verts, %u indices",
           _directory, mesh.vertices.size(), mesh.indices.size());

  return mesh;
}

void* FileSystem::LoadImageFile(const char* _directory, int& _width, int& _height)
{
  int channels;
  stbi_uc* image = stbi_load(_directory, &_width, &_height, &channels, STBI_rgb_alpha);
  assert (image != nullptr);
  return image;
}

void FileSystem::DestroyImageFile(void* _image)
{
  stbi_image_free(_image);
}

