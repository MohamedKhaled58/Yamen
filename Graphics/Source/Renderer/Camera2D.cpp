#include "Graphics/Renderer/Camera2D.h"

namespace Yamen::Graphics {

    Camera2D::Camera2D(float width, float height)
        : m_Position(0.0f, 0.0f)
        , m_Zoom(1.0f)
        , m_Rotation(0.0f)
        , m_ViewportWidth(width)
        , m_ViewportHeight(height)
        , m_ViewMatrix(1.0f)
        , m_ProjectionMatrix(1.0f)
    {
        RecalculateViewMatrix();
        RecalculateProjectionMatrix();
    }

    void Camera2D::SetPosition(const glm::vec2& position) {
        m_Position = position;
        RecalculateViewMatrix();
    }

    void Camera2D::SetZoom(float zoom) {
        m_Zoom = glm::max(0.1f, zoom); // Prevent zero/negative zoom
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

    glm::vec2 Camera2D::ScreenToWorld(const glm::vec2& screenPos) const {
        // Convert screen coordinates to NDC
        glm::vec2 ndc;
        ndc.x = (2.0f * screenPos.x) / m_ViewportWidth - 1.0f;
        ndc.y = 1.0f - (2.0f * screenPos.y) / m_ViewportHeight;

        // Transform by inverse view-projection
        glm::mat4 invViewProj = glm::inverse(GetViewProjectionMatrix());
        glm::vec4 worldPos = invViewProj * glm::vec4(ndc, 0.0f, 1.0f);

        return glm::vec2(worldPos.x, worldPos.y);
    }

    void Camera2D::RecalculateViewMatrix() {
        // Create transformation matrix
        glm::mat4 transform = glm::mat4(1.0f);
        
        // Apply transformations in order: translate -> rotate -> scale
        transform = glm::translate(transform, glm::vec3(m_Position, 0.0f));
        transform = glm::rotate(transform, m_Rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        transform = glm::scale(transform, glm::vec3(m_Zoom, m_Zoom, 1.0f));

        // View matrix is inverse of camera transform
        m_ViewMatrix = glm::inverse(transform);
    }

    void Camera2D::RecalculateProjectionMatrix() {
        // Orthographic projection centered at origin
        float left = -m_ViewportWidth * 0.5f;
        float right = m_ViewportWidth * 0.5f;
        float bottom = -m_ViewportHeight * 0.5f;
        float top = m_ViewportHeight * 0.5f;

        m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
    }

} // namespace Yamen::Graphics
