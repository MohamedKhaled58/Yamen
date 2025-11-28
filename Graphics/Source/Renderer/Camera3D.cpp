#include "Graphics/Renderer/Camera3D.h"

namespace Yamen::Graphics {

Camera3D::Camera3D(float fov, float aspectRatio, float nearPlane,
                   float farPlane)
    : m_Position(0.0f, 0.0f, 0.0f), m_Rotation(0.0f, 0.0f, 0.0f),
      m_Forward(0.0f, 0.0f, -1.0f), m_Right(1.0f, 0.0f, 0.0f),
      m_Up(0.0f, 1.0f, 0.0f), m_FOV(fov), m_AspectRatio(aspectRatio),
      m_NearPlane(nearPlane), m_FarPlane(farPlane), m_ViewMatrix(1.0f),
      m_ProjectionMatrix(1.0f) {
  RecalculateViewMatrix();
  RecalculateProjectionMatrix();
}

void Camera3D::SetPosition(const glm::vec3 &position) {
  m_Position = position;
  RecalculateViewMatrix();
}

void Camera3D::SetRotation(const glm::vec3 &rotation) {
  m_Rotation = rotation;
  UpdateVectors();
  RecalculateViewMatrix();
}

void Camera3D::SetTransform(const glm::vec3 &position,
                            const glm::quat &rotation) {
  m_Position = position;

  // Calculate vectors directly from quaternion
  // Using -Z forward for LookAt convention (matches CameraController)
  m_Forward = glm::normalize(
      rotation * glm::vec3(0.0f, 0.0f, -1.0f)); // -Z Forward (LookAt)
  m_Right = glm::normalize(rotation * glm::vec3(1.0f, 0.0f, 0.0f)); // +X Right
  m_Up = glm::normalize(rotation * glm::vec3(0.0f, 1.0f, 0.0f));    // +Y Up

  RecalculateViewMatrix();
}

void Camera3D::LookAt(const glm::vec3 &eye, const glm::vec3 &center,
                      const glm::vec3 &up) {
  m_Position = eye;
  m_Forward = glm::normalize(center - eye);
  m_Right = glm::normalize(glm::cross(m_Forward, up));
  m_Up = glm::normalize(glm::cross(m_Right, m_Forward));

  // Update Rotation (Pitch, Yaw) from Forward vector
  // forward.y = sin(pitch) => pitch = asin(forward.y)
  m_Rotation.x = asin(m_Forward.y);

  // forward.z = sin(yaw) * cos(pitch)
  // forward.x = cos(yaw) * cos(pitch)
  // tan(yaw) = forward.z / forward.x
  // Use atan2(z, x)
  m_Rotation.y = atan2(m_Forward.z, m_Forward.x);

  m_Rotation.z = 0.0f; // Roll is usually 0 for LookAt

  RecalculateViewMatrix();
}

void Camera3D::SetFOV(float fov) {
  m_FOV = fov;
  RecalculateProjectionMatrix();
}

void Camera3D::SetAspectRatio(float aspectRatio) {
  m_AspectRatio = aspectRatio;
  RecalculateProjectionMatrix();
}

void Camera3D::SetClipPlanes(float nearPlane, float farPlane) {
  m_NearPlane = nearPlane;
  m_FarPlane = farPlane;
  RecalculateProjectionMatrix();
}

void Camera3D::RecalculateViewMatrix() {
  m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Forward, m_Up);
}

void Camera3D::RecalculateProjectionMatrix() {
  m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_AspectRatio,
                                        m_NearPlane, m_FarPlane);
}

void Camera3D::UpdateVectors() {
  // Calculate forward vector from Euler angles (Pitch and Yaw only, ignore Roll
  // for FPS camera)
  glm::vec3 forward;
  forward.x = cos(m_Rotation.y) * cos(m_Rotation.x);
  forward.y = sin(m_Rotation.x);
  forward.z = sin(m_Rotation.y) * cos(m_Rotation.x);
  m_Forward = glm::normalize(forward);

  // Calculate right and up vectors
  m_Right = glm::normalize(glm::cross(m_Forward, glm::vec3(0.0f, 1.0f, 0.0f)));
  m_Up = glm::normalize(glm::cross(m_Right, m_Forward));
}

} // namespace Yamen::Graphics
