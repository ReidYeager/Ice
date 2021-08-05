
#ifndef ICE_CORE_TRANSFORM_H_
#define ICE_CORE_TRANSFORM_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct transform_t
{
  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;
  glm::mat4 matrix;

  glm::mat4 GetMatrix()
  {
    scale = glm::vec3(1.0f);
    matrix = glm::translate(glm::mat4(1.0f), position);
    glm::fquat rotationQuaternion = {glm::radians(rotation)};
    matrix *= glm::mat4_cast(rotationQuaternion);
    matrix = glm::scale(matrix, scale);
    return matrix;
  }
};

#endif // !CORE_TRANSFORM_H
