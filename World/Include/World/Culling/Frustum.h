#pragma once

#include <glm/glm.hpp>
#include <array>

namespace Yamen::World {

    struct Plane {
        glm::vec3 Normal;
        float Distance;

        Plane() = default;
        Plane(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3);
        
        float GetSignedDistance(const glm::vec3& point) const;
    };

    class Frustum {
    public:
        Frustum() = default;

        // Update frustum from view-projection matrix
        void Update(const glm::mat4& viewProj);

        // Intersection tests
        bool ContainsPoint(const glm::vec3& point) const;
        bool ContainsSphere(const glm::vec3& center, float radius) const;
        bool ContainsBox(const glm::vec3& min, const glm::vec3& max) const;

    private:
        std::array<Plane, 6> m_Planes;
        
        enum {
            LEFT = 0,
            RIGHT,
            BOTTOM,
            TOP,
            NEAR_PLANE,
            FAR_PLANE
        };
    };

} // namespace Yamen::World
