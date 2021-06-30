
#include "core/camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

IceCamera::IceCamera()
{
  view = glm::mat4(1);
  projection = glm::mat4(1);

  position = glm::vec3(0);
  rotation = glm::vec3(0);
}

IceCamera::~IceCamera()
{
  
}

glm::mat4 IceCamera::GetViewMatrix()
{
  // TODO : add roll
  m_forward.x = (float)(glm::cos(glm::radians(rotation.y)) * glm::cos(glm::radians(rotation.x)));
  m_forward.y = (float)(glm::sin(glm::radians(rotation.x)));
  m_forward.z = (float)(glm::sin(glm::radians(rotation.y)) * glm::cos(glm::radians(rotation.x)));

  view = glm::lookAt(position, position + m_forward, {0.0f, 1.0f, 0.0f});

  return view;
}

glm::mat4 IceCamera::UpdateProjection(float _screenRatio, float _vertFov /*= 90.0f*/)
{
  m_vertFieldOfView = _vertFov;

  projection = glm::perspective(glm::radians(m_vertFieldOfView), _screenRatio,
                                m_minRenderDist, m_maxRenderDist);
  projection[1][1] *= -1; // Flip the rendered y-axis

  return projection;
}

glm::vec3 IceCamera::GetForward()
{
  return m_forward;
}

glm::vec3 IceCamera::GetRight()
{
  return glm::normalize(glm::cross(m_forward, { 0.f, 1.f, 0.f }));
  //return right;
}

glm::vec3 IceCamera::GetUp()
{
  return glm::cross(m_forward, right);
}

void IceCamera::Rotate(glm::vec3 _rotationAmount)
{
  rotation += _rotationAmount;
}

void IceCamera::SetRotation(glm::vec3 _newRotation)
{
  rotation = _newRotation;
}

void IceCamera::ClampPitch(f32 _max, f32 _min)
{
  if(rotation.x > _max)
    rotation.x = _max;
  else if (rotation.x < _min)
    rotation.x = _min;
}

