#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Yamen::Graphics {

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
        void SetPosition(const glm::vec2& position);
        void SetPosition(float x, float y) { SetPosition(glm::vec2(x, y)); }

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
        const glm::vec2& GetPosition() const { return m_Position; }
        float GetZoom() const { return m_Zoom; }
        float GetRotation() const { return m_Rotation; }

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

        /**
         * @brief Convert screen coordinates to world coordinates
         */
        glm::vec2 ScreenToWorld(const glm::vec2& screenPos) const;

    private:
        void RecalculateViewMatrix();
        void RecalculateProjectionMatrix();

        glm::vec2 m_Position;
        float m_Zoom;
        float m_Rotation;
        float m_ViewportWidth;
        float m_ViewportHeight;

        glm::mat4 m_ViewMatrix;
        glm::mat4 m_ProjectionMatrix;
    };

} // namespace Yamen::Graphics
