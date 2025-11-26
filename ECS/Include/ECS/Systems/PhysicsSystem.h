#pragma once

#include "ECS/ISystem.h"
#include "ECS/Scene.h"
#include "ECS/Components/CoreComponents.h"
#include "ECS/Components/PhysicsComponents.h"
#include <glm/glm.hpp>
#include <vector>

namespace Yamen::ECS {

    /**
     * @brief Professional Physics System
     * 
     * Features:
     * - Semi-implicit Euler integration
     * - AABB & Sphere collision detection
     * - Impulse-based collision resolution
     * - Gravity and Drag
     */
    class PhysicsSystem : public ISystem {
    public:
        struct Manifold {
            entt::entity EntityA;
            entt::entity EntityB;
            glm::vec3 Normal;
            float Penetration;
        };

        void OnInit(Scene* scene) override;
        void OnUpdate(Scene* scene, float deltaTime) override;
        void OnRender(Scene* scene) override;
        void OnShutdown(Scene* scene) override;

        int GetPriority() const override { return 200; } // Update after scripts, before rendering
        const char* GetName() const override { return "PhysicsSystem"; }

        // Settings
        glm::vec3 Gravity = glm::vec3(0.0f, -9.81f, 0.0f);
        int SubSteps = 1; // Increase for better stability at cost of performance

    private:
        void IntegrateForces(Scene* scene, float dt);
        void IntegrateVelocity(Scene* scene, float dt);
        void DetectCollisions(Scene* scene, std::vector<Manifold>& manifolds);
        void ResolveCollisions(Scene* scene, const std::vector<Manifold>& manifolds);
        
        // Collision primitives
        bool CheckCollision(const TransformComponent& tA, const ColliderComponent& cA,
                          const TransformComponent& tB, const ColliderComponent& cB,
                          Manifold& manifold);
                          
        bool IntersectSphereSphere(const glm::vec3& posA, float radiusA,
                                 const glm::vec3& posB, float radiusB,
                                 Manifold& manifold);
                                 
        bool IntersectAABBAABB(const glm::vec3& minA, const glm::vec3& maxA,
                             const glm::vec3& minB, const glm::vec3& maxB,
                             Manifold& manifold);

        bool IntersectSphereAABB(const glm::vec3& spherePos, float sphereRadius,
                               const glm::vec3& boxMin, const glm::vec3& boxMax,
                               Manifold& manifold);
    };

} // namespace Yamen::ECS
