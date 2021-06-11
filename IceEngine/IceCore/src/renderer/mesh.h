
#ifndef RENDERER_MESH_H
#define RENDERER_MESH_H 1

#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan.h>

#include "defines.h"

struct vertex_t
{
  glm::vec3 position;
  glm::vec2 uv;
  glm::vec3 normal;

  // Required for hash mapping
  // Compares the attributes of other against itself
  bool operator==(const vertex_t& other) const
  {
    return position == other.position && normal == other.normal && uv == other.uv;
  }
};

// TODO : Make a better hash function http://www.azillionmonkeys.com/qed/hash.html
// Used to map vertices into an unordered array during mesh building
namespace std {
  template<> struct hash<vertex_t> {
    size_t operator()(vertex_t const& vertex) const {
      return ((hash<glm::vec3>()(vertex.position) ^
        (hash<glm::vec2>()(vertex.uv) << 1)) >> 1) ^
        (hash<glm::vec3>()(vertex.normal) << 1);
    }
  };
}

struct mesh_t
{
  std::vector<vertex_t> vertices;
  std::vector<u32> indices;

  VkBuffer vertexBuffer;
  VkBuffer indexBuffer;
};

#endif // !RENDERER_MESH_H
