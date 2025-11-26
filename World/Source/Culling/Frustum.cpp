#include "World/Culling/Frustum.h"

namespace Yamen::World {

    Plane::Plane(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3) {
        Normal = glm::normalize(glm::cross(p2 - p1, p3 - p1));
        Distance = -glm::dot(Normal, p1);
    }

    float Plane::GetSignedDistance(const glm::vec3& point) const {
        return glm::dot(Normal, point) + Distance;
    }

    void Frustum::Update(const glm::mat4& viewProj) {
        // Extract planes from View-Projection matrix
        // Gribb/Hartmann method
        
        // Left plane
        m_Planes[LEFT].Normal.x = viewProj[0][3] + viewProj[0][0];
        m_Planes[LEFT].Normal.y = viewProj[1][3] + viewProj[1][0];
        m_Planes[LEFT].Normal.z = viewProj[2][3] + viewProj[2][0];
        m_Planes[LEFT].Distance = viewProj[3][3] + viewProj[3][0];

        // Right plane
        m_Planes[RIGHT].Normal.x = viewProj[0][3] - viewProj[0][0];
        m_Planes[RIGHT].Normal.y = viewProj[1][3] - viewProj[1][0];
        m_Planes[RIGHT].Normal.z = viewProj[2][3] - viewProj[2][0];
        m_Planes[RIGHT].Distance = viewProj[3][3] - viewProj[3][0];

        // Bottom plane
        m_Planes[BOTTOM].Normal.x = viewProj[0][3] + viewProj[0][1];
        m_Planes[BOTTOM].Normal.y = viewProj[1][3] + viewProj[1][1];
        m_Planes[BOTTOM].Normal.z = viewProj[2][3] + viewProj[2][1];
        m_Planes[BOTTOM].Distance = viewProj[3][3] + viewProj[3][1];

        // Top plane
        m_Planes[TOP].Normal.x = viewProj[0][3] - viewProj[0][1];
        m_Planes[TOP].Normal.y = viewProj[1][3] - viewProj[1][1];
        m_Planes[TOP].Normal.z = viewProj[2][3] - viewProj[2][1];
        m_Planes[TOP].Distance = viewProj[3][3] - viewProj[3][1];

        // Near plane
        m_Planes[NEAR_PLANE].Normal.x = viewProj[0][3] + viewProj[0][2];
        m_Planes[NEAR_PLANE].Normal.y = viewProj[1][3] + viewProj[1][2];
        m_Planes[NEAR_PLANE].Normal.z = viewProj[2][3] + viewProj[2][2];
        m_Planes[NEAR_PLANE].Distance = viewProj[3][3] + viewProj[3][2];

        // Far plane
        m_Planes[FAR_PLANE].Normal.x = viewProj[0][3] - viewProj[0][2];
        m_Planes[FAR_PLANE].Normal.y = viewProj[1][3] - viewProj[1][2];
        m_Planes[FAR_PLANE].Normal.z = viewProj[2][3] - viewProj[2][2];
        m_Planes[FAR_PLANE].Distance = viewProj[3][3] - viewProj[3][2];

        // Normalize planes
        for (auto& plane : m_Planes) {
            float length = glm::length(plane.Normal);
            plane.Normal /= length;
            plane.Distance /= length;
        }
    }

    bool Frustum::ContainsPoint(const glm::vec3& point) const {
        for (const auto& plane : m_Planes) {
            if (plane.GetSignedDistance(point) < 0) {
                return false;
            }
        }
        return true;
    }

    bool Frustum::ContainsSphere(const glm::vec3& center, float radius) const {
        for (const auto& plane : m_Planes) {
            if (plane.GetSignedDistance(center) < -radius) {
                return false;
            }
        }
        return true;
    }

    bool Frustum::ContainsBox(const glm::vec3& min, const glm::vec3& max) const {
        // Check if any point of the box is inside all planes
        // Optimization: check positive vertex against plane
        
        for (const auto& plane : m_Planes) {
            glm::vec3 positiveVertex = min;
            if (plane.Normal.x >= 0) positiveVertex.x = max.x;
            if (plane.Normal.y >= 0) positiveVertex.y = max.y;
            if (plane.Normal.z >= 0) positiveVertex.z = max.z;

            if (plane.GetSignedDistance(positiveVertex) < 0) {
                return false;
            }
        }
        return true;
    }

} // namespace Yamen::World
