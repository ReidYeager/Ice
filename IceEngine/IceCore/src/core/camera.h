
#ifndef CORE_CAMERA_H
#define CORE_CAMERA_H 1

#include "defines.h"

#include <glm/glm.hpp>

class IceCamera
{
public:
  glm::vec3 position;

private:
  glm::mat4 projection;
  glm::mat4 view;

  glm::vec3 rotation;
  //glm::vec3 scale;

  glm::vec3 m_forward;
  glm::vec3 right;

  float m_vertFieldOfView = 90.0f;
  float m_minRenderDist = 0.1f;
  float m_maxRenderDist = 10.0f;

public:
  IceCamera();
  ~IceCamera();
  // void MouseMoveCallback();

  void Update();
  glm::mat4 GetViewMatrix();
  glm::mat4 UpdateProjection(float _screenRatio, float _vertFieldOfView = 90.0f);

  glm::vec3 GetForward();
  glm::vec3 GetRight();
  glm::vec3 GetUp();

  void Rotate(glm::vec3 _rotationAmount);
  void SetRotation(glm::vec3 _newRotation);
  void ClampPitch(i32 _max, i32 _min);

};

#endif // !CORE_CAMERA_H
