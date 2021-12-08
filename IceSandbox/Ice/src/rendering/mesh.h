
#ifndef ICE_RENDERING_MESH_H_
#define ICE_RENDERING_MESH_H_

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan.h>

#include <vector>
#include <string>

struct iceVertex
{
  glm::vec3 position;
  glm::vec2 uv;
  glm::vec3 normal;

  // Required for hash mapping
  // Compares the attributes of other against itself
  bool operator==(const iceVertex& other) const
  {
    return position == other.position && normal == other.normal && uv == other.uv;
  }

  static VkVertexInputBindingDescription GetBindingDescription()
  {
    VkVertexInputBindingDescription desc = {};
    desc.stride = sizeof(iceVertex);
    desc.binding = 0;
    desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return desc;
  }

  static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions()
  {
    std::vector<VkVertexInputAttributeDescription> attribs(3);
    // Position
    attribs[0].binding = 0;
    attribs[0].location = 0;
    attribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribs[0].offset = offsetof(iceVertex, position);
    // UV
    attribs[2].binding = 0;
    attribs[2].location = 1;
    attribs[2].format = VK_FORMAT_R32G32_SFLOAT;
    attribs[2].offset = offsetof(iceVertex, uv);
    // normal
    attribs[1].binding = 0;
    attribs[1].location = 2;
    attribs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribs[1].offset = offsetof(iceVertex, normal);

    return attribs;
  }
};

// NOTE : Make a better hash function http://www.azillionmonkeys.com/qed/hash.html
// Used to map vertices into an unordered array during mesh building
namespace std {
  template<> struct hash<iceVertex> {
    size_t operator()(iceVertex const& vertex) const {
      return ((hash<glm::vec3>()(vertex.position) ^
        (hash<glm::vec2>()(vertex.uv) << 1)) >> 1) ^
        (hash<glm::vec3>()(vertex.normal) << 1);
    }
  };
}

struct mesh_t
{
  std::string directory;

  std::vector<iceVertex> vertices;
  std::vector<u32> indices;

  //IceBuffer vertexBuffer;
  //IceBuffer indexBuffer;
};

#endif // !define ICE_RENDERING_MESH_H_
