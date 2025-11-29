#pragma once

#include "Core/Math/Math.h"
#include "ECS/Components/CoreComponents.h"
#include "ECS/Components/PhysicsComponents.h"
#include "ECS/Components/XPBDComponents.h"
#include "ECS/ISystem.h"
#include "ECS/Scene.h"
#include <unordered_map>
#include <vector>


namespace Yamen::ECS {

using namespace Yamen::Core;

/**
 * @brief XPBD (Extended Position Based Dynamics) Solver
 *
 * Professional physics solver using position-based constraints with compliance.
 *
 * Features:
 * - Time-step independent constraint solving
 * - Compliance-based stiffness control
 * - Unified constraint framework
 * - Warm-starting with Lagrange multipliers
 * - Multi-iteration Gauss-Seidel solver
 *
 * Algorithm per frame:
 * 1. Predict positions: x_pred = x + v*dt + (1/m)*F_ext*dt²
 * 2. Generate collision constraints
 * 3. For each substep:
 *    - For each solver iteration:
 *      - Solve all constraints
 *    - Update velocities: v = (x_new - x_old) / dt
 *    - Apply friction
 * 4. Update transforms
 */
class XPBDSolver : public ISystem {
public:
  XPBDSolver();
  ~XPBDSolver() override;

  void OnInit(Scene *scene) override;
  void OnUpdate(Scene *scene, float deltaTime) override;
  void OnRender(Scene *scene) override;
  void OnShutdown(Scene *scene) override;

  int GetPriority() const override { return 200; }
  const char *GetName() const override { return "XPBDSolver"; }

  // Configuration
  vec3 Gravity = vec3(0.0f, -9.81f, 0.0f);
  int SubSteps = 4;             // Number of substeps per frame
  int SolverIterations = 10;    // Constraint solver iterations per substep
  float SleepThreshold = 0.01f; // Velocity threshold for sleeping
  float SleepTime = 0.5f;       // Time below threshold before sleeping
  bool EnableSleeping = true;
  bool EnableWarmStarting = true;

  // Statistics
  struct Stats {
    int ActiveParticles = 0;
    int SleepingParticles = 0;
    int ActiveConstraints = 0;
    int ContactConstraints = 0;
    float SolveTime = 0.0f;
    float CollisionTime = 0.0f;
  };
  Stats GetStats() const { return m_Stats; }

private:
  // Simulation steps
  void PredictPositions(Scene *scene, float dt);
  void GenerateCollisionConstraints(Scene *scene);
  void SolveConstraints(Scene *scene, float dt);
  void UpdateVelocities(Scene *scene, float dt);
  void ApplyFriction(Scene *scene, float dt);
  void UpdateTransforms(Scene *scene);
  void UpdateSleeping(Scene *scene, float dt);

  // Constraint solving
  void SolveDistanceConstraint(Scene *scene, DistanceConstraint &constraint,
                               float dt);
  void SolveContactConstraint(Scene *scene, ContactConstraint &constraint,
                              float dt);
  void SolveBendingConstraint(Scene *scene, BendingConstraint &constraint,
                              float dt);
  void SolveVolumeConstraint(Scene *scene, VolumeConstraint &constraint,
                             float dt);
  void SolveShapeMatchingConstraint(Scene *scene,
                                    ShapeMatchingConstraint &constraint,
                                    float dt);
  void SolveBallSocketConstraint(Scene *scene, BallSocketConstraint &constraint,
                                 float dt);
  void SolveHingeConstraint(Scene *scene, HingeConstraint &constraint,
                            float dt);
  void SolveSliderConstraint(Scene *scene, SliderConstraint &constraint,
                             float dt);

  // Collision detection
  struct CollisionPair {
    entt::entity EntityA;
    entt::entity EntityB;
  };
  void BroadPhaseCollision(Scene *scene, std::vector<CollisionPair> &pairs);
  bool NarrowPhaseCollision(Scene *scene, const CollisionPair &pair,
                            ContactConstraint &contact);

  // Helpers
  void ApplyPositionDelta(XPBDParticleComponent &particle, const vec3 &delta);
  float ComputeGeneralizedInverseMass(const XPBDParticleComponent &p1,
                                      const XPBDParticleComponent &p2,
                                      const vec3 &grad1, const vec3 &grad2);

  // Temporary contact constraints (cleared each frame)
  std::vector<ContactConstraint> m_ContactConstraints;

  // Statistics
  Stats m_Stats;
};

} // namespace Yamen::ECS
