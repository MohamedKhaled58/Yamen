#include "Graphics/Renderer/Camera2D.h"

namespace Yamen::Graphics {

using namespace Yamen::Core;

Camera2D::Camera2D(float width, float height)
    : m_Position(0.0f, 0.0f), m_Zoom(1.0f), m_Rotation(0.0f),
      m_ViewportWidth(width), m_ViewportHeight(height), m_ViewMatrix(1.0f),
      m_ProjectionMatrix(1.0f) {
  RecalculateViewMatrix();
  RecalculateProjectionMatrix();
}

void Camera2D::SetPosition(const vec2 &position) {
  m_Position = position;
  RecalculateViewMatrix();
}

void Camera2D::SetZoom(float zoom) {
  m_Zoom = Math::Max(0.1f, zoom); // Prevent zero/negative zoom
  RecalculateViewMatrix();
}

void Camera2D::SetRotation(float rotation) {
  m_Rotation = rotation;
  RecalculateViewMatrix();
}

void Camera2D::SetViewportSize(float width, float height) {
  m_ViewportWidth = width;
  m_ViewportHeight = height;
  RecalculateProjectionMatrix();
}

vec2 Camera2D::ScreenToWorld(const vec2 &screenPos) const {
  // Convert screen coordinates to NDC
  vec2 ndc;
  ndc.x = (2.0f * screenPos.x) / m_ViewportWidth - 1.0f;
  ndc.y = 1.0f - (2.0f * screenPos.y) / m_ViewportHeight;

  // Transform by inverse view-projection
  mat4 invViewProj = Math::Inverse(GetViewProjectionMatrix());
  vec4 worldPos = invViewProj * vec4(ndc.x, ndc.y, 0.0f, 1.0f);

  return vec2(worldPos.x, worldPos.y);
}

void Camera2D::RecalculateViewMatrix() {
  // Create transformation matrix
  mat4 transform = mat4(1.0f);

  // Apply transformations in order: translate -> rotate -> scale
  transform =
      Math::Translate(transform, vec3(m_Position.x, m_Position.y, 0.0f));
  transform = Math::Rotate(transform, m_Rotation, vec3(0.0f, 0.0f, 1.0f));
  transform = Math::Scale(transform, vec3(m_Zoom, m_Zoom, 1.0f));

  // View matrix is inverse of camera transform
  m_ViewMatrix = Math::Inverse(transform);
}

void Camera2D::RecalculateProjectionMatrix() {
  // Orthographic projection centered at origin
  float left = -m_ViewportWidth * 0.5f;
  float right = m_ViewportWidth * 0.5f;
  float bottom = -m_ViewportHeight * 0.5f;
  float top = m_ViewportHeight * 0.5f;

  m_ProjectionMatrix = Math::Ortho(left, right, bottom, top, -1.0f, 1.0f);
}

} // namespace Yamen::Graphics
