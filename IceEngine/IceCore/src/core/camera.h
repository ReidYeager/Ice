
#ifndef ICE_CORE_CAMERA_H_
#define ICE_CORE_CAMERA_H_

#include "defines.h"

#include <glm/glm.hpp>

class ICE_API IceCamera
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

  float vertFieldOfView = 90.0f;
  float minRenderDist = 0.1f;
  float maxRenderDist = 10.0f;

public:
  IceCamera();
  ~IceCamera();
  // void MouseMoveCallback();

  void Update();
  glm::mat4 GetViewMatrix();
  glm::mat4 GetProjectionMatrix();
  void SetProjection(float _screenRatio, float _vertFieldOfView = 90.0f);

  glm::vec3 GetForward();
  glm::vec3 GetRight();
  glm::vec3 GetUp();

  void Rotate(glm::vec3 _rotationAmount);
  void SetRotation(glm::vec3 _newRotation);
  void ClampPitch(f32 _max, f32 _min);

};

#endif // !CORE_CAMERA_H
