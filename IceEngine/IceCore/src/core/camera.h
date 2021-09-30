
#ifndef ICE_CORE_CAMERA_H_
#define ICE_CORE_CAMERA_H_

#include "defines.h"

#include <glm/glm.hpp>

// TODO : Create a camera (projection) component to replace this
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
  glm::vec3 m_up = { 0.0f, 1.0f, 0.0f };

  float verticalFieldOfView = 90.0f;
  float minRenderDist = 0.1f;
  float maxRenderDist = 100.0f;

public:
  IceCamera();
  ~IceCamera();
  // void MouseMoveCallback();

  // Creates and returns the camera's transform matrix
  glm::mat4 GetViewMatrix();
  // Returns the camera's projection matrix
  glm::mat4 GetProjectionMatrix();
  // Creates a projection matrix from the input settings
  void SetProjection(float _screenRatio, float _vertFieldOfView = 45.0f);

  // Direction vector getters
  glm::vec3 GetForward();
  glm::vec3 GetRight();
  glm::vec3 GetUp();

  void Rotate(glm::vec3 _rotationAmount);
  void SetRotation(glm::vec3 _newRotation);
  void ClampPitch(f32 _max, f32 _min);

};

#endif // !CORE_CAMERA_H
