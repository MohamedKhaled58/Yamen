#include "Graphics/Renderer/Camera3D.h"
#include <cmath>

namespace Yamen::Graphics {

using namespace Yamen::Core;

Camera3D::Camera3D(float fov, float aspectRatio, float nearPlane,
                   float farPlane)
    : m_Position(0.0f, 0.0f, 0.0f), m_Rotation(0.0f, 0.0f, 0.0f),
      m_Forward(0.0f, 0.0f, 1.0f),
      m_Right(1.0f, 0.0f, 0.0f), // Forward is +Z in DX LH
      m_Up(0.0f, 1.0f, 0.0f), m_FOV(fov), m_AspectRatio(aspectRatio),
      m_NearPlane(nearPlane), m_FarPlane(farPlane), m_ViewMatrix(1.0f),
      m_ProjectionMatrix(1.0f) {
  RecalculateViewMatrix();
  RecalculateProjectionMatrix();
}

void Camera3D::SetPosition(const vec3 &position) {
  m_Position = position;
  RecalculateViewMatrix();
}

void Camera3D::SetRotation(const vec3 &rotation) {
  m_Rotation = rotation;
  UpdateVectors();
  RecalculateViewMatrix();
}

void Camera3D::SetTransform(const vec3 &position, const quat &rotation) {
  m_Position = position;

  // Calculate vectors directly from quaternion
  // Using +Z forward for DX LH
  vec3 forward(0.0f, 0.0f, 1.0f);
  vec3 right(1.0f, 0.0f, 0.0f);
  vec3 up(0.0f, 1.0f, 0.0f);

  // Rotate vectors by quaternion
  XMVECTOR q = XMLoadFloat4(&rotation);

  m_Forward = vec3(XMVector3Rotate(XMLoadFloat3(&forward), q));
  m_Right = vec3(XMVector3Rotate(XMLoadFloat3(&right), q));
  m_Up = vec3(XMVector3Rotate(XMLoadFloat3(&up), q));

  RecalculateViewMatrix();
}

void Camera3D::LookAt(const vec3 &eye, const vec3 &center, const vec3 &up) {
  m_Position = eye;
  m_Forward = Math::Normalize(center - eye);
  m_Right = Math::Normalize(
      Math::Cross(up, m_Forward)); // LH Cross product order might be different?
  // DX LH: Cross(Up, Forward) -> Right? No.
  // Standard: Right = Cross(Up, Forward) is Left Handed?
  // Let's stick to Math::LookAt implementation which uses XMMatrixLookAtLH

  // Just update vectors for consistency if needed, but ViewMatrix is what
  // matters. Actually LookAt updates ViewMatrix directly.
  m_ViewMatrix = Math::LookAtLH(eye, center, up);

  // Extract Forward/Right/Up from ViewMatrix if needed, or just recalculate
  // them. For now, let's trust LookAt to set the matrix. But we need to update
  // m_Rotation for consistency if we switch back to Euler. This is complex.
  // Let's just set the matrix.
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
  m_ViewMatrix = Math::LookAtLH(m_Position, m_Position + m_Forward, m_Up);
}

void Camera3D::RecalculateProjectionMatrix() {
  m_ProjectionMatrix = Math::Perspective(Math::Radians(m_FOV), m_AspectRatio,
                                         m_NearPlane, m_FarPlane);
}

void Camera3D::UpdateVectors() {
  // Calculate forward vector from Euler angles
  // DX LH: Z is forward.
  vec3 forward;
  forward.x = cos(m_Rotation.y) * cos(m_Rotation.x);
  forward.y = sin(m_Rotation.x);
  forward.z = sin(m_Rotation.y) * cos(m_Rotation.x);
  m_Forward = Math::Normalize(forward);

  m_Right = Math::Normalize(Math::Cross(
      vec3(0.0f, 1.0f, 0.0f), m_Forward)); // Up x Forward = Right in LH?
  m_Up = Math::Normalize(Math::Cross(m_Forward, m_Right));
}

} // namespace Yamen::Graphics
