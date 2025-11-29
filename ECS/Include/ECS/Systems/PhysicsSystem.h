#pragma once

#include "ECS/Components/CoreComponents.h"
#include "ECS/Components/PhysicsComponents.h"
#include "ECS/ISystem.h"
#include "ECS/Scene.h"
#include <Core/Math/Math.h>
#include <vector>


namespace Yamen::ECS {

using namespace Yamen::Core;

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
    vec3 Normal;
    float Penetration;
  };

  void OnInit(Scene *scene) override;
  void OnUpdate(Scene *scene, float deltaTime) override;
  void OnRender(Scene *scene) override;
  void OnShutdown(Scene *scene) override;

  int GetPriority() const override {
    return 200;
  } // Update after scripts, before rendering
  const char *GetName() const override { return "PhysicsSystem"; }

  // Settings
  vec3 Gravity = vec3(0.0f, -9.81f, 0.0f);
  int SubSteps = 1; // Increase for better stability at cost of performance

private:
  void IntegrateForces(Scene *scene, float dt);
  void IntegrateVelocity(Scene *scene, float dt);
  void DetectCollisions(Scene *scene, std::vector<Manifold> &manifolds);
  void ResolveCollisions(Scene *scene, const std::vector<Manifold> &manifolds);

  // Collision primitives
  bool CheckCollision(const TransformComponent &tA, const ColliderComponent &cA,
                      const TransformComponent &tB, const ColliderComponent &cB,
                      Manifold &manifold);

  bool IntersectSphereSphere(const vec3 &posA, float radiusA, const vec3 &posB,
                             float radiusB, Manifold &manifold);

  bool IntersectAABBAABB(const vec3 &minA, const vec3 &maxA, const vec3 &minB,
                         const vec3 &maxB, Manifold &manifold);

  bool IntersectSphereAABB(const vec3 &spherePos, float sphereRadius,
                           const vec3 &boxMin, const vec3 &boxMax,
                           Manifold &manifold);
};

} // namespace Yamen::ECS
