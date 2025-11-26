#pragma once

#include "Graphics/RHI/GraphicsDevice.h"

// Force GLM to use 0 to 1 depth range (DirectX) instead of -1 to 1 (OpenGL)
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Yamen::Graphics {

    /**
     * @brief 3D perspective camera
     */
    class Camera3D {
    public:
        Camera3D(float fov = 60.0f, float aspectRatio = 16.0f / 9.0f, float nearPlane = 0.1f, float farPlane = 1000.0f);
        ~Camera3D() = default;

        /**
         * @brief Set camera position
         */
        void SetPosition(const glm::vec3& position);
        void SetPosition(float x, float y, float z) { SetPosition(glm::vec3(x, y, z)); }

        /**
         * @brief Set camera rotation (Euler angles in radians)
         */
        void SetRotation(const glm::vec3& rotation);
        void SetRotation(float pitch, float yaw, float roll) { SetRotation(glm::vec3(pitch, yaw, roll)); }

        /**
         * @brief Set camera transform directly (Position + Rotation Quaternion)
         */
        void SetTransform(const glm::vec3& position, const glm::quat& rotation);

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
        const glm::vec3& GetPosition() const { return m_Position; }
        const glm::vec3& GetRotation() const { return m_Rotation; }
        const glm::vec3& GetForward() const { return m_Forward; }
        const glm::vec3& GetRight() const { return m_Right; }
        const glm::vec3& GetUp() const { return m_Up; }
        float GetFOV() const { return m_FOV; }

        /**
         * @brief Get view matrix
         */
        const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }

        /**
         * @brief Get projection matrix
         */
        const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }

        /**
         * @brief Get view-projection matrix
         */
        glm::mat4 GetViewProjectionMatrix() const { return m_ProjectionMatrix * m_ViewMatrix; }

    private:
        void RecalculateViewMatrix();
        void RecalculateProjectionMatrix();
        void UpdateVectors();

        glm::vec3 m_Position;
        glm::vec3 m_Rotation; // Pitch, Yaw, Roll
        glm::vec3 m_Forward;
        glm::vec3 m_Right;
        glm::vec3 m_Up;

        float m_FOV;
        float m_AspectRatio;
        float m_NearPlane;
        float m_FarPlane;

        glm::mat4 m_ViewMatrix;
        glm::mat4 m_ProjectionMatrix;
    };

} // namespace Yamen::Graphics
