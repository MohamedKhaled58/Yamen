#pragma once

#include <Core/Math/Math.h>

namespace Yamen::Graphics {

using namespace Yamen::Core;

/**
 * @brief 2D orthographic camera
 *
 * Provides view and projection matrices for 2D rendering.
 */
class Camera2D {
public:
  Camera2D(float width, float height);
  ~Camera2D() = default;

  /**
   * @brief Set camera position
   */
  void SetPosition(const vec2 &position);
  void SetPosition(float x, float y) { SetPosition(vec2(x, y)); }

  /**
   * @brief Set camera zoom (1.0 = normal, >1.0 = zoomed in, <1.0 = zoomed out)
   */
  void SetZoom(float zoom);

  /**
   * @brief Set camera rotation (in radians)
   */
  void SetRotation(float rotation);

  /**
   * @brief Set viewport size
   */
  void SetViewportSize(float width, float height);

  /**
   * @brief Get camera properties
   */
  const vec2 &GetPosition() const { return m_Position; }
  float GetZoom() const { return m_Zoom; }
  float GetRotation() const { return m_Rotation; }

  /**
   * @brief Get view matrix
   */
  const mat4 &GetViewMatrix() const { return m_ViewMatrix; }

  /**
   * @brief Get projection matrix
   */
  const mat4 &GetProjectionMatrix() const { return m_ProjectionMatrix; }

  /**
   * @brief Get view-projection matrix
   */
  mat4 GetViewProjectionMatrix() const {
      return m_ViewMatrix * m_ProjectionMatrix;
  }

  /**
   * @brief Convert screen coordinates to world coordinates
   */
  vec2 ScreenToWorld(const vec2 &screenPos) const;

private:
  void RecalculateViewMatrix();
  void RecalculateProjectionMatrix();

  vec2 m_Position;
  float m_Zoom;
  float m_Rotation;
  float m_ViewportWidth;
  float m_ViewportHeight;

  mat4 m_ViewMatrix;
  mat4 m_ProjectionMatrix;
};

} // namespace Yamen::Graphics
