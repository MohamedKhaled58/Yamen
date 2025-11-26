#include "ECS/Systems/PhysicsSystem.h"
#include "ECS/Components.h"
#include <Core/Logging/Logger.h>
#include <glm/gtx/norm.hpp>

namespace Yamen::ECS {

    void PhysicsSystem::OnInit(Scene* scene) {
        YAMEN_CORE_INFO("PhysicsSystem initialized");
    }

    void PhysicsSystem::OnUpdate(Scene* scene, float deltaTime) {
        if (!scene) return;

        float dt = deltaTime / (float)SubSteps;

        for (int step = 0; step < SubSteps; ++step) {
            IntegrateForces(scene, dt);
            
            std::vector<Manifold> manifolds;
            DetectCollisions(scene, manifolds);
            ResolveCollisions(scene, manifolds);
            
            IntegrateVelocity(scene, dt);
        }
    }

    void PhysicsSystem::OnRender(Scene* scene) {
        // TODO: Debug rendering of colliders (wireframes)
    }

    void PhysicsSystem::OnShutdown(Scene* scene) {
        // Cleanup
    }

    void PhysicsSystem::IntegrateForces(Scene* scene, float dt) {
        auto view = scene->Registry().view<RigidBodyComponent>();
        for (auto entity : view) {
            auto& body = view.get<RigidBodyComponent>(entity);
            
            if (body.Type != BodyType::Dynamic || body.IsSleeping) continue;

            // Apply Gravity
            if (body.UseGravity) {
                body.AddForce(Gravity * body.Mass);
            }

            // F = ma -> a = F/m
            glm::vec3 acceleration = body.Force * body.GetInverseMass();
            
            // Integrate Velocity: v += a * dt
            body.Velocity += acceleration * dt;

            // Apply Drag (simplified linear drag)
            body.Velocity *= (1.0f - body.LinearDrag);

            // Clear forces
            body.Force = glm::vec3(0.0f);
            body.Torque = glm::vec3(0.0f);
        }
    }

    void PhysicsSystem::IntegrateVelocity(Scene* scene, float dt) {
        auto view = scene->Registry().view<TransformComponent, RigidBodyComponent>();
        for (auto entity : view) {
            auto [transform, body] = view.get<TransformComponent, RigidBodyComponent>(entity);
            
            if (body.Type == BodyType::Static || body.IsSleeping) continue;

            // Integrate Position: p += v * dt
            transform.Translation += body.Velocity * dt;
        }
    }

    void PhysicsSystem::DetectCollisions(Scene* scene, std::vector<Manifold>& manifolds) {
        auto view = scene->Registry().view<TransformComponent, ColliderComponent>();
        
        // Naive O(N^2) broadphase for now - optimize with QuadTree later
        for (auto it1 = view.begin(); it1 != view.end(); ++it1) {
            entt::entity e1 = *it1;
            for (auto it2 = std::next(it1); it2 != view.end(); ++it2) {
                entt::entity e2 = *it2;

                auto [t1, c1] = view.get<TransformComponent, ColliderComponent>(e1);
                auto [t2, c2] = view.get<TransformComponent, ColliderComponent>(e2);

                // Skip if both are static (no collision response needed)
                bool b1Static = true;
                bool b2Static = true;
                
                if (scene->Registry().all_of<RigidBodyComponent>(e1)) {
                    b1Static = scene->Registry().get<RigidBodyComponent>(e1).Type == BodyType::Static;
                }
                if (scene->Registry().all_of<RigidBodyComponent>(e2)) {
                    b2Static = scene->Registry().get<RigidBodyComponent>(e2).Type == BodyType::Static;
                }

                if (b1Static && b2Static) continue;

                Manifold manifold;
                manifold.EntityA = e1;
                manifold.EntityB = e2;

                if (CheckCollision(t1, c1, t2, c2, manifold)) {
                    manifolds.push_back(manifold);
                }
            }
        }
    }

    void PhysicsSystem::ResolveCollisions(Scene* scene, const std::vector<Manifold>& manifolds) {
        for (const auto& m : manifolds) {
            RigidBodyComponent* b1 = scene->Registry().try_get<RigidBodyComponent>(m.EntityA);
            RigidBodyComponent* b2 = scene->Registry().try_get<RigidBodyComponent>(m.EntityB);
            TransformComponent& t1 = scene->Registry().get<TransformComponent>(m.EntityA);
            TransformComponent& t2 = scene->Registry().get<TransformComponent>(m.EntityB);

            if (!b1 && !b2) continue;

            float invMass1 = b1 ? b1->GetInverseMass() : 0.0f;
            float invMass2 = b2 ? b2->GetInverseMass() : 0.0f;
            float totalInvMass = invMass1 + invMass2;

            if (totalInvMass == 0.0f) continue;

            // Separate bodies (positional correction)
            glm::vec3 correction = m.Normal * (m.Penetration / totalInvMass);
            if (invMass1 > 0.0f) t1.Translation -= correction * invMass1;
            if (invMass2 > 0.0f) t2.Translation += correction * invMass2;

            // Impulse resolution
            glm::vec3 rv = (b2 ? b2->Velocity : glm::vec3(0.0f)) - (b1 ? b1->Velocity : glm::vec3(0.0f));
            float velAlongNormal = glm::dot(rv, m.Normal);

            if (velAlongNormal > 0) continue; // Moving away

            // Calculate restitution (bounciness)
            float e = 0.5f; // Average bounciness for now
            
            float j = -(1.0f + e) * velAlongNormal;
            j /= totalInvMass;

            glm::vec3 impulse = m.Normal * j;
            
            if (b1 && b1->Type == BodyType::Dynamic) b1->Velocity -= impulse * invMass1;
            if (b2 && b2->Type == BodyType::Dynamic) b2->Velocity += impulse * invMass2;
        }
    }

    bool PhysicsSystem::CheckCollision(const TransformComponent& tA, const ColliderComponent& cA,
                                     const TransformComponent& tB, const ColliderComponent& cB,
                                     Manifold& manifold) {
        // Dispatch to specific intersection tests based on types
        if (cA.Type == ColliderType::Sphere && cB.Type == ColliderType::Sphere) {
            const auto& sA = std::get<SphereCollider>(cA.Shape);
            const auto& sB = std::get<SphereCollider>(cB.Shape);
            return IntersectSphereSphere(tA.Translation + sA.Offset, sA.Radius,
                                       tB.Translation + sB.Offset, sB.Radius, manifold);
        }
        else if (cA.Type == ColliderType::Box && cB.Type == ColliderType::Box) {
            const auto& bA = std::get<BoxCollider>(cA.Shape);
            const auto& bB = std::get<BoxCollider>(cB.Shape);
            
            // AABB approximation
            glm::vec3 minA = tA.Translation + bA.Offset - bA.HalfExtents;
            glm::vec3 maxA = tA.Translation + bA.Offset + bA.HalfExtents;
            glm::vec3 minB = tB.Translation + bB.Offset - bB.HalfExtents;
            glm::vec3 maxB = tB.Translation + bB.Offset + bB.HalfExtents;
            
            return IntersectAABBAABB(minA, maxA, minB, maxB, manifold);
        }
        else if (cA.Type == ColliderType::Sphere && cB.Type == ColliderType::Box) {
            const auto& sA = std::get<SphereCollider>(cA.Shape);
            const auto& bB = std::get<BoxCollider>(cB.Shape);
            
            glm::vec3 minB = tB.Translation + bB.Offset - bB.HalfExtents;
            glm::vec3 maxB = tB.Translation + bB.Offset + bB.HalfExtents;
            
            return IntersectSphereAABB(tA.Translation + sA.Offset, sA.Radius, minB, maxB, manifold);
        }
        else if (cA.Type == ColliderType::Box && cB.Type == ColliderType::Sphere) {
            const auto& bA = std::get<BoxCollider>(cA.Shape);
            const auto& sB = std::get<SphereCollider>(cB.Shape);
            
            glm::vec3 minA = tA.Translation + bA.Offset - bA.HalfExtents;
            glm::vec3 maxA = tA.Translation + bA.Offset + bA.HalfExtents;
            
            // Flip normal for Box-Sphere
            bool collision = IntersectSphereAABB(tB.Translation + sB.Offset, sB.Radius, minA, maxA, manifold);
            if (collision) {
                manifold.Normal = -manifold.Normal;
            }
            return collision;
        }
        
        return false;
    }

    bool PhysicsSystem::IntersectSphereSphere(const glm::vec3& posA, float radiusA,
                                            const glm::vec3& posB, float radiusB,
                                            Manifold& manifold) {
        glm::vec3 n = posB - posA;
        float distSq = glm::length2(n);
        float radiusSum = radiusA + radiusB;

        if (distSq > radiusSum * radiusSum) return false;

        float dist = std::sqrt(distSq);
        
        if (dist == 0.0f) {
            manifold.Normal = glm::vec3(0, 1, 0);
            manifold.Penetration = radiusSum;
        } else {
            manifold.Normal = n / dist;
            manifold.Penetration = radiusSum - dist;
        }
        
        return true;
    }

    bool PhysicsSystem::IntersectAABBAABB(const glm::vec3& minA, const glm::vec3& maxA,
                                        const glm::vec3& minB, const glm::vec3& maxB,
                                        Manifold& manifold) {
        glm::vec3 centerA = (minA + maxA) * 0.5f;
        glm::vec3 centerB = (minB + maxB) * 0.5f;
        
        glm::vec3 halfExtentsA = (maxA - minA) * 0.5f;
        glm::vec3 halfExtentsB = (maxB - minB) * 0.5f;
        
        glm::vec3 d = centerB - centerA;
        
        float overlapX = (halfExtentsA.x + halfExtentsB.x) - std::abs(d.x);
        if (overlapX <= 0) return false;
        
        float overlapY = (halfExtentsA.y + halfExtentsB.y) - std::abs(d.y);
        if (overlapY <= 0) return false;
        
        float overlapZ = (halfExtentsA.z + halfExtentsB.z) - std::abs(d.z);
        if (overlapZ <= 0) return false;
        
        // Find minimum overlap axis
        if (overlapX < overlapY && overlapX < overlapZ) {
            manifold.Penetration = overlapX;
            manifold.Normal = glm::vec3(d.x > 0 ? 1 : -1, 0, 0);
        } else if (overlapY < overlapZ) {
            manifold.Penetration = overlapY;
            manifold.Normal = glm::vec3(0, d.y > 0 ? 1 : -1, 0);
        } else {
            manifold.Penetration = overlapZ;
            manifold.Normal = glm::vec3(0, 0, d.z > 0 ? 1 : -1);
        }
        
        return true;
    }

    bool PhysicsSystem::IntersectSphereAABB(const glm::vec3& spherePos, float sphereRadius,
                                          const glm::vec3& boxMin, const glm::vec3& boxMax,
                                          Manifold& manifold) {
        // Find closest point on AABB to sphere center
        glm::vec3 closestPoint = glm::clamp(spherePos, boxMin, boxMax);
        
        glm::vec3 n = spherePos - closestPoint;
        float distSq = glm::length2(n);
        
        if (distSq > sphereRadius * sphereRadius) return false;
        
        float dist = std::sqrt(distSq);
        
        // Sphere center is inside AABB
        if (dist == 0.0f) {
            // Find minimum penetration axis to push sphere out
            glm::vec3 center = (boxMin + boxMax) * 0.5f;
            glm::vec3 halfExtents = (boxMax - boxMin) * 0.5f;
            glm::vec3 d = spherePos - center;
            
            float overlapX = halfExtents.x - std::abs(d.x);
            float overlapY = halfExtents.y - std::abs(d.y);
            float overlapZ = halfExtents.z - std::abs(d.z);
            
            if (overlapX < overlapY && overlapX < overlapZ) {
                manifold.Penetration = overlapX + sphereRadius;
                manifold.Normal = glm::vec3(d.x > 0 ? 1 : -1, 0, 0);
            } else if (overlapY < overlapZ) {
                manifold.Penetration = overlapY + sphereRadius;
                manifold.Normal = glm::vec3(0, d.y > 0 ? 1 : -1, 0);
            } else {
                manifold.Penetration = overlapZ + sphereRadius;
                manifold.Normal = glm::vec3(0, 0, d.z > 0 ? 1 : -1);
            }
        } else {
            manifold.Normal = n / dist;
            manifold.Penetration = sphereRadius - dist;
        }
        
        return true;
    }

} // namespace Yamen::ECS
