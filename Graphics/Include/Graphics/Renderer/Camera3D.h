#pragma once

#include "Graphics/RHI/GraphicsDevice.h"
#include <Core/Math/Math.h>

namespace Yamen::Graphics {

using namespace Yamen::Core;

/**
 * @brief 3D perspective camera
 */
class Camera3D {
public:
  Camera3D(float fov = 60.0f, float aspectRatio = 16.0f / 9.0f,
           float nearPlane = 0.1f, float farPlane = 1000.0f);
  ~Camera3D() = default;

  /**
   * @brief Set camera position
   */
  void SetPosition(const vec3 &position);
  void SetPosition(float x, float y, float z) { SetPosition(vec3(x, y, z)); }

  /**
   * @brief Set camera rotation (Euler angles in radians)
   */
  void SetRotation(const vec3 &rotation);
  void SetRotation(float pitch, float yaw, float roll) {
    SetRotation(vec3(pitch, yaw, roll));
  }

  /**
   * @brief Set camera transform directly (Position + Rotation Quaternion)
   */
  void SetTransform(const vec3 &position, const quat &rotation);

  /**
   * @brief Set camera to look at a target point
   */
  void LookAt(const vec3 &eye, const vec3 &center,
              const vec3 &up = vec3(0.0f, 1.0f, 0.0f));

  /**
   * @brief Set field of view (in degrees)
   */
  void SetFOV(float fov);

  /**
   * @brief Set aspect ratio
   */
  void SetAspectRatio(float aspectRatio);

  /**
   * @brief Set near and far planes
   */
  void SetClipPlanes(float nearPlane, float farPlane);

  /**
   * @brief Get camera properties
   */
  const vec3 &GetPosition() const { return m_Position; }
  const vec3 &GetRotation() const { return m_Rotation; }
  const vec3 &GetForward() const { return m_Forward; }
  const vec3 &GetRight() const { return m_Right; }
  const vec3 &GetUp() const { return m_Up; }
  float GetFOV() const { return m_FOV; }

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

private:
  void RecalculateViewMatrix();
  void RecalculateProjectionMatrix();
  void UpdateVectors();

  vec3 m_Position;
  vec3 m_Rotation; // Pitch, Yaw, Roll
  vec3 m_Forward;
  vec3 m_Right;
  vec3 m_Up;

  float m_FOV;
  float m_AspectRatio;
  float m_NearPlane;
  float m_FarPlane;

  mat4 m_ViewMatrix;
  mat4 m_ProjectionMatrix;
};

} // namespace Yamen::Graphics
