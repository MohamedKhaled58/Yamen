#include "ECS/Systems/XPBDSolver.h"
#include "ECS/Components.h"
#include <Core/Logging/Logger.h>
#include <Core/Math/Math.h>
#include <algorithm>
#include <chrono>

namespace Yamen::ECS {

using namespace Core::Math;

XPBDSolver::XPBDSolver() {}

XPBDSolver::~XPBDSolver() {}

void XPBDSolver::OnInit(Scene *scene) {
  YAMEN_CORE_INFO("XPBD Solver initialized");
  YAMEN_CORE_INFO("  SubSteps: {}", SubSteps);
  YAMEN_CORE_INFO("  Solver Iterations: {}", SolverIterations);
}

void XPBDSolver::OnUpdate(Scene *scene, float deltaTime) {
  if (!scene)
    return;

  auto startTime = std::chrono::high_resolution_clock::now();

  // Reset statistics
  m_Stats = Stats();

  // Substep the simulation for stability
  float dt = deltaTime / static_cast<float>(SubSteps);

  for (int substep = 0; substep < SubSteps; ++substep) {
    // 1. Predict positions using external forces
    PredictPositions(scene, dt);

    // 2. Generate collision constraints
    auto collisionStart = std::chrono::high_resolution_clock::now();
    GenerateCollisionConstraints(scene);
    auto collisionEnd = std::chrono::high_resolution_clock::now();
    m_Stats.CollisionTime =
        std::chrono::duration<float, std::milli>(collisionEnd - collisionStart)
            .count();

    // 3. Solve all constraints iteratively
    auto solveStart = std::chrono::high_resolution_clock::now();
    SolveConstraints(scene, dt);
    auto solveEnd = std::chrono::high_resolution_clock::now();
    m_Stats.SolveTime =
        std::chrono::duration<float, std::milli>(solveEnd - solveStart).count();

    // 4. Update velocities from position changes
    UpdateVelocities(scene, dt);

    // 5. Apply friction
    ApplyFriction(scene, dt);
  }

  // 6. Update transform components from particle positions
  UpdateTransforms(scene);

  // 7. Update sleeping state
  if (EnableSleeping) {
    UpdateSleeping(scene, deltaTime);
  }

  // Clear temporary contact constraints
  m_ContactConstraints.clear();
}

void XPBDSolver::OnRender(Scene *scene) {
  // Debug rendering handled by PhysicsDebugRenderer
}

void XPBDSolver::OnShutdown(Scene *scene) { m_ContactConstraints.clear(); }

void XPBDSolver::PredictPositions(Scene *scene, float dt) {
  auto view = scene->Registry().view<XPBDParticleComponent>();

  m_Stats.ActiveParticles = 0;
  m_Stats.SleepingParticles = 0;

  for (auto entity : view) {
    auto &particle = view.get<XPBDParticleComponent>(entity);

    if (particle.IsSleeping) {
      m_Stats.SleepingParticles++;
      continue;
    }

    m_Stats.ActiveParticles++;

    if (particle.IsStatic()) {
      continue;
    }

    // Store previous position for velocity calculation
    particle.PreviousPosition = particle.Position;

    // Apply gravity
    particle.ExternalForce += Gravity * particle.GetMass();

    // Semi-implicit Euler: v += (F/m) * dt
    vec3 acceleration = particle.ExternalForce * particle.InverseMass;
    particle.Velocity += acceleration * dt;

    // Predict position: x_new = x + v * dt
    particle.Position += particle.Velocity * dt;

    // Clear external forces
    particle.ExternalForce = vec3(0.0f);
  }
}

void XPBDSolver::GenerateCollisionConstraints(Scene *scene) {
  // Broad phase: find potential collision pairs
  std::vector<CollisionPair> pairs;
  BroadPhaseCollision(scene, pairs);

  // Narrow phase: generate contact constraints
  for (const auto &pair : pairs) {
    ContactConstraint contact;
    if (NarrowPhaseCollision(scene, pair, contact)) {
      m_ContactConstraints.push_back(contact);
    }
  }

  m_Stats.ContactConstraints = static_cast<int>(m_ContactConstraints.size());
}

void XPBDSolver::SolveConstraints(Scene *scene, float dt) {
  auto constraintView = scene->Registry().view<XPBDConstraintComponent>();

  m_Stats.ActiveConstraints =
      static_cast<int>(constraintView.size()) + m_Stats.ContactConstraints;

  // Gauss-Seidel iterations
  for (int iteration = 0; iteration < SolverIterations; ++iteration) {
    // Solve persistent constraints
    for (auto entity : constraintView) {
      auto &constraintComp =
          constraintView.get<XPBDConstraintComponent>(entity);

      if (!constraintComp.GetBase()->Active)
        continue;

      // Dispatch to appropriate solver based on constraint type
      std::visit(
          [&](auto &constraint) {
            using T = std::decay_t<decltype(constraint)>;
            if constexpr (std::is_same_v<T, DistanceConstraint>) {
              SolveDistanceConstraint(scene, constraint, dt);
            } else if constexpr (std::is_same_v<T, ContactConstraint>) {
              SolveContactConstraint(scene, constraint, dt);
            } else if constexpr (std::is_same_v<T, BendingConstraint>) {
              SolveBendingConstraint(scene, constraint, dt);
            } else if constexpr (std::is_same_v<T, VolumeConstraint>) {
              SolveVolumeConstraint(scene, constraint, dt);
            } else if constexpr (std::is_same_v<T, ShapeMatchingConstraint>) {
              SolveShapeMatchingConstraint(scene, constraint, dt);
            } else if constexpr (std::is_same_v<T, BallSocketConstraint>) {
              SolveBallSocketConstraint(scene, constraint, dt);
            } else if constexpr (std::is_same_v<T, HingeConstraint>) {
              SolveHingeConstraint(scene, constraint, dt);
            } else if constexpr (std::is_same_v<T, SliderConstraint>) {
              SolveSliderConstraint(scene, constraint, dt);
            }
          },
          constraintComp.Constraint);
    }

    // Solve contact constraints
    for (auto &contact : m_ContactConstraints) {
      SolveContactConstraint(scene, contact, dt);
    }
  }
}

void XPBDSolver::SolveDistanceConstraint(Scene *scene,
                                         DistanceConstraint &constraint,
                                         float dt) {
  auto &registry = scene->Registry();

  auto *p1 = registry.try_get<XPBDParticleComponent>(constraint.ParticleA);
  auto *p2 = registry.try_get<XPBDParticleComponent>(constraint.ParticleB);

  if (!p1 || !p2)
    return;
  if (p1->IsSleeping && p2->IsSleeping)
    return;

  // Compute constraint value: C = |p1 - p2| - rest_length
  vec3 delta = p1->Position - p2->Position;
  float currentLength = Math::Length(delta);

  if (currentLength < 1e-6f)
    return; // Avoid division by zero

  float C = currentLength - constraint.RestLength;

  // For rope constraints, only enforce if stretched
  if (constraint.IsRope && C < 0.0f)
    return;

  // Compute gradient: grad_C = (p1 - p2) / |p1 - p2|
  vec3 grad = delta / currentLength;

  // Compute generalized inverse mass
  float w1 = p1->InverseMass;
  float w2 = p2->InverseMass;
  float w = w1 + w2;

  if (w < 1e-6f)
    return; // Both static

  // XPBD: alpha = compliance / dt^2
  float alpha = constraint.Compliance / (dt * dt);

  // Compute delta lambda
  float deltaLambda = (-C - alpha * constraint.Lambda) / (w + alpha);

  // Warm starting: use previous lambda
  if (!EnableWarmStarting) {
    constraint.Lambda = 0.0f;
  }

  // Update lambda
  constraint.Lambda += deltaLambda;

  // Apply position corrections
  vec3 correction = grad * deltaLambda;
  if (w1 > 0.0f)
    p1->Position += correction * w1;
  if (w2 > 0.0f)
    p2->Position -= correction * w2;
}

void XPBDSolver::SolveContactConstraint(Scene *scene,
                                        ContactConstraint &constraint,
                                        float dt) {
  auto &registry = scene->Registry();

  auto *p1 = registry.try_get<XPBDParticleComponent>(constraint.ParticleA);
  auto *p2 = registry.try_get<XPBDParticleComponent>(constraint.ParticleB);

  if (!p1 || !p2)
    return;
  if (p1->IsSleeping && p2->IsSleeping)
    return;

  // Compute constraint value: C = dot(p1 - p2, normal) - penetration
  vec3 delta = p1->Position - p2->Position;
  float C = Math::Dot(delta, constraint.Normal) - constraint.Penetration;

  // Only enforce if penetrating
  if (C >= 0.0f)
    return;

  // Gradient is the normal
  vec3 grad = constraint.Normal;

  // Compute generalized inverse mass
  float w1 = p1->InverseMass;
  float w2 = p2->InverseMass;
  float w = w1 + w2;

  if (w < 1e-6f)
    return;

  // Contact constraints are typically rigid (zero compliance)
  float alpha = constraint.Compliance / (dt * dt);

  // Compute delta lambda (unilateral constraint: lambda >= 0)
  float deltaLambda = (-C - alpha * constraint.Lambda) / (w + alpha);

  // Clamp to ensure non-penetration only (no pulling)
  float newLambda = std::max(0.0f, constraint.Lambda + deltaLambda);
  deltaLambda = newLambda - constraint.Lambda;
  constraint.Lambda = newLambda;

  // Apply position corrections
  vec3 correction = grad * deltaLambda;
  if (w1 > 0.0f)
    p1->Position += correction * w1;
  if (w2 > 0.0f)
    p2->Position -= correction * w2;
}

void XPBDSolver::SolveBendingConstraint(Scene *scene,
                                        BendingConstraint &constraint,
                                        float dt) {
  auto &registry = scene->Registry();

  auto *p0 = registry.try_get<XPBDParticleComponent>(constraint.Particle0);
  auto *p1 = registry.try_get<XPBDParticleComponent>(constraint.Particle1);
  auto *p2 = registry.try_get<XPBDParticleComponent>(constraint.Particle2);
  auto *p3 = registry.try_get<XPBDParticleComponent>(constraint.Particle3);

  if (!p0 || !p1 || !p2 || !p3)
    return;

  // Compute normals of the two triangles
  vec3 e0 = p1->Position - p0->Position;
  vec3 e1 = p2->Position - p0->Position;
  vec3 e2 = p3->Position - p0->Position;

  vec3 n1 = Math::Cross(e0, e1);
  vec3 n2 = Math::Cross(e0, e2);

  float len1 = Math::Length(n1);
  float len2 = Math::Length(n2);

  if (len1 < 1e-6f || len2 < 1e-6f)
    return;

  n1 /= len1;
  n2 /= len2;

  // Compute current angle
  float cosAngle = Math::Clamp(Math::Dot(n1, n2), -1.0f, 1.0f);
  float currentAngle = std::acos(cosAngle);

  // Constraint: C = current_angle - rest_angle
  float C = currentAngle - constraint.RestAngle;

  // Simplified gradient approximation (full derivation is complex)
  // For small angles, we can use a linear approximation
  float alpha = constraint.Compliance / (dt * dt);
  float w =
      p0->InverseMass + p1->InverseMass + p2->InverseMass + p3->InverseMass;

  if (w < 1e-6f)
    return;

  float deltaLambda = -C / (w + alpha);
  constraint.Lambda += deltaLambda;

  // Apply corrections (simplified)
  vec3 correction = Math::Cross(n1, n2) * deltaLambda * 0.25f;
  if (p0->InverseMass > 0.0f)
    p0->Position += correction * p0->InverseMass;
  if (p1->InverseMass > 0.0f)
    p1->Position += correction * p1->InverseMass;
  if (p2->InverseMass > 0.0f)
    p2->Position -= correction * p2->InverseMass;
  if (p3->InverseMass > 0.0f)
    p3->Position -= correction * p3->InverseMass;
}

void XPBDSolver::SolveVolumeConstraint(Scene *scene,
                                       VolumeConstraint &constraint, float dt) {
  auto &registry = scene->Registry();

  auto *p0 = registry.try_get<XPBDParticleComponent>(constraint.Particle0);
  auto *p1 = registry.try_get<XPBDParticleComponent>(constraint.Particle1);
  auto *p2 = registry.try_get<XPBDParticleComponent>(constraint.Particle2);
  auto *p3 = registry.try_get<XPBDParticleComponent>(constraint.Particle3);

  if (!p0 || !p1 || !p2 || !p3)
    return;

  // Compute current volume: V = 1/6 * dot(p1-p0, cross(p2-p0, p3-p0))
  vec3 e1 = p1->Position - p0->Position;
  vec3 e2 = p2->Position - p0->Position;
  vec3 e3 = p3->Position - p0->Position;

  float currentVolume = Math::Dot(e1, Math::Cross(e2, e3)) / 6.0f;

  // Constraint: C = current_volume - rest_volume
  float C = currentVolume - constraint.RestVolume;

  // Compute gradients
  vec3 grad0 = -Math::Cross(e2, e3) / 6.0f;
  vec3 grad1 = Math::Cross(e2, e3) / 6.0f;
  vec3 grad2 = Math::Cross(e3, e1) / 6.0f;
  vec3 grad3 = Math::Cross(e1, e2) / 6.0f;

  // Compute generalized inverse mass
  float w = p0->InverseMass * Math::LengthSq(grad0) +
            p1->InverseMass * Math::LengthSq(grad1) +
            p2->InverseMass * Math::LengthSq(grad2) +
            p3->InverseMass * Math::LengthSq(grad3);

  if (w < 1e-6f)
    return;

  float alpha = constraint.Compliance / (dt * dt);
  float deltaLambda = (-C - alpha * constraint.Lambda) / (w + alpha);
  constraint.Lambda += deltaLambda;

  // Apply corrections
  if (p0->InverseMass > 0.0f)
    p0->Position += grad0 * deltaLambda * p0->InverseMass;
  if (p1->InverseMass > 0.0f)
    p1->Position += grad1 * deltaLambda * p1->InverseMass;
  if (p2->InverseMass > 0.0f)
    p2->Position += grad2 * deltaLambda * p2->InverseMass;
  if (p3->InverseMass > 0.0f)
    p3->Position += grad3 * deltaLambda * p3->InverseMass;
}

void XPBDSolver::SolveShapeMatchingConstraint(
    Scene *scene, ShapeMatchingConstraint &constraint, float dt) {
  // Shape matching is more complex and requires computing optimal rotation
  // This is a simplified version

  auto &registry = scene->Registry();

  if (constraint.Particles.empty())
    return;

  // Compute current center of mass
  vec3 currentCOM(0.0f);
  float totalMass = 0.0f;

  for (auto entity : constraint.Particles) {
    auto *p = registry.try_get<XPBDParticleComponent>(entity);
    if (!p)
      continue;
    float mass = p->GetMass();
    currentCOM += p->Position * mass;
    totalMass += mass;
  }

  if (totalMass < 1e-6f)
    return;
  currentCOM /= totalMass;

  // Compute goal positions (simplified: no rotation matching)
  size_t i = 0;
  for (auto entity : constraint.Particles) {
    auto *p = registry.try_get<XPBDParticleComponent>(entity);
    if (!p || i >= constraint.RestPositions.size())
      continue;

    vec3 goalPos = currentCOM + constraint.RestPositions[i];
    vec3 delta = goalPos - p->Position;

    // Apply correction with compliance
    float alpha = constraint.Compliance / (dt * dt);
    float w = p->InverseMass;
    if (w < 1e-6f)
      continue;

    float deltaLambda = Math::Length(delta) / (w + alpha);
    vec3 correction = Math::Normalize(delta) * deltaLambda;

    p->Position += correction * w;
    i++;
  }
}

void XPBDSolver::SolveBallSocketConstraint(Scene *scene,
                                           BallSocketConstraint &constraint,
                                           float dt) {
  auto &registry = scene->Registry();

  auto *p1 = registry.try_get<XPBDParticleComponent>(constraint.ParticleA);
  auto *p2 = registry.try_get<XPBDParticleComponent>(constraint.ParticleB);

  if (!p1 || !p2)
    return;

  // Constraint: C = p1 - p2 (3D constraint)
  vec3 C = p1->Position - p2->Position;

  float w1 = p1->InverseMass;
  float w2 = p2->InverseMass;
  float w = w1 + w2;

  if (w < 1e-6f)
    return;

  float alpha = constraint.Compliance / (dt * dt);

  // Solve for each axis independently
  for (int axis = 0; axis < 3; ++axis) {
    float c = C[axis];
    float deltaLambda = -c / (w + alpha);

    vec3 correction(0.0f);
    correction[axis] = deltaLambda;

    if (w1 > 0.0f)
      p1->Position += correction * w1;
    if (w2 > 0.0f)
      p2->Position -= correction * w2;
  }
}

void XPBDSolver::SolveHingeConstraint(Scene *scene, HingeConstraint &constraint,
                                      float dt) {
  // Hinge constraint is complex, requires position + orientation constraints
  // Simplified implementation: just constrain positions like ball-socket
  auto &registry = scene->Registry();

  auto *p1 = registry.try_get<XPBDParticleComponent>(constraint.ParticleA);
  auto *p2 = registry.try_get<XPBDParticleComponent>(constraint.ParticleB);

  if (!p1 || !p2)
    return;

  vec3 C = p1->Position - p2->Position;
  float w = p1->InverseMass + p2->InverseMass;
  if (w < 1e-6f)
    return;

  float alpha = constraint.Compliance / (dt * dt);
  vec3 correction = C / (w + alpha);

  if (p1->InverseMass > 0.0f)
    p1->Position -= correction * p1->InverseMass;
  if (p2->InverseMass > 0.0f)
    p2->Position += correction * p2->InverseMass;
}

void XPBDSolver::SolveSliderConstraint(Scene *scene,
                                       SliderConstraint &constraint, float dt) {
  // Slider constraint: constrain motion perpendicular to slide axis
  auto &registry = scene->Registry();

  auto *p1 = registry.try_get<XPBDParticleComponent>(constraint.ParticleA);
  auto *p2 = registry.try_get<XPBDParticleComponent>(constraint.ParticleB);

  if (!p1 || !p2)
    return;

  vec3 delta = p1->Position - p2->Position;

  // Project delta onto plane perpendicular to slide axis
  vec3 slideAxis = Math::Normalize(constraint.SlideAxis);
  float projection = Math::Dot(delta, slideAxis);
  vec3 perpendicular = delta - slideAxis * projection;

  float w = p1->InverseMass + p2->InverseMass;
  if (w < 1e-6f)
    return;

  float alpha = constraint.Compliance / (dt * dt);
  vec3 correction = perpendicular / (w + alpha);

  if (p1->InverseMass > 0.0f)
    p1->Position -= correction * p1->InverseMass;
  if (p2->InverseMass > 0.0f)
    p2->Position += correction * p2->InverseMass;
}

void XPBDSolver::UpdateVelocities(Scene *scene, float dt) {
  auto view = scene->Registry().view<XPBDParticleComponent>();

  for (auto entity : view) {
    auto &particle = view.get<XPBDParticleComponent>(entity);

    if (particle.IsSleeping || particle.IsStatic())
      continue;

    // Update velocity from position change: v = (x_new - x_old) / dt
    particle.Velocity = (particle.Position - particle.PreviousPosition) / dt;
  }
}

void XPBDSolver::ApplyFriction(Scene *scene, float dt) {
  // Apply friction to contact constraints
  for (auto &contact : m_ContactConstraints) {
    auto &registry = scene->Registry();

    auto *p1 = registry.try_get<XPBDParticleComponent>(contact.ParticleA);
    auto *p2 = registry.try_get<XPBDParticleComponent>(contact.ParticleB);

    if (!p1 || !p2)
      continue;

    // Compute relative velocity
    vec3 relVel = p1->Velocity - p2->Velocity;

    // Tangential velocity (perpendicular to normal)
    vec3 tangentVel =
        relVel - contact.Normal * Math::Dot(relVel, contact.Normal);
    float tangentSpeed = Math::Length(tangentVel);

    if (tangentSpeed < 1e-6f)
      continue;

    vec3 tangentDir = tangentVel / tangentSpeed;

    // Coulomb friction
    float normalForce = contact.Lambda / (dt * dt);
    float maxFriction = contact.Friction * normalForce * dt;

    // Clamp friction impulse
    float frictionImpulse = std::min(tangentSpeed, maxFriction);

    vec3 frictionDelta = tangentDir * frictionImpulse;

    float w1 = p1->InverseMass;
    float w2 = p2->InverseMass;
    float w = w1 + w2;

    if (w < 1e-6f)
      continue;

    if (w1 > 0.0f)
      p1->Velocity -= frictionDelta * (w1 / w);
    if (w2 > 0.0f)
      p2->Velocity += frictionDelta * (w2 / w);
  }
}

void XPBDSolver::UpdateTransforms(Scene *scene) {
  auto view =
      scene->Registry().view<TransformComponent, XPBDParticleComponent>();

  for (auto entity : view) {
    auto &transform = view.get<TransformComponent>(entity);
    auto &particle = view.get<XPBDParticleComponent>(entity);

    // Update transform position from particle
    transform.Translation = particle.Position;
  }
}

void XPBDSolver::UpdateSleeping(Scene *scene, float dt) {
  auto view = scene->Registry().view<XPBDParticleComponent>();

  for (auto entity : view) {
    auto &particle = view.get<XPBDParticleComponent>(entity);

    if (particle.IsStatic())
      continue;

    float speed = Math::Length(particle.Velocity);

    if (speed < SleepThreshold) {
      particle.SleepTimer += dt;
      if (particle.SleepTimer > SleepTime) {
        particle.IsSleeping = true;
        particle.Velocity = vec3(0.0f);
      }
    } else {
      particle.SleepTimer = 0.0f;
      particle.IsSleeping = false;
    }
  }
}

void XPBDSolver::BroadPhaseCollision(Scene *scene,
                                     std::vector<CollisionPair> &pairs) {
  // Simple O(N^2) broad phase for now
  // TODO: Implement spatial hashing for better performance

  auto view =
      scene->Registry()
          .view<TransformComponent, ColliderComponent, XPBDParticleComponent>();

  for (auto it1 = view.begin(); it1 != view.end(); ++it1) {
    for (auto it2 = std::next(it1); it2 != view.end(); ++it2) {
      entt::entity e1 = *it1;
      entt::entity e2 = *it2;

      auto &p1 = view.get<XPBDParticleComponent>(e1);
      auto &p2 = view.get<XPBDParticleComponent>(e2);

      // Skip if both sleeping or both static
      if ((p1.IsSleeping && p2.IsSleeping) ||
          (p1.IsStatic() && p2.IsStatic())) {
        continue;
      }

      pairs.push_back({e1, e2});
    }
  }
}

bool XPBDSolver::NarrowPhaseCollision(Scene *scene, const CollisionPair &pair,
                                      ContactConstraint &contact) {
  auto &registry = scene->Registry();

  auto *t1 = registry.try_get<TransformComponent>(pair.EntityA);
  auto *t2 = registry.try_get<TransformComponent>(pair.EntityB);
  auto *c1 = registry.try_get<ColliderComponent>(pair.EntityA);
  auto *c2 = registry.try_get<ColliderComponent>(pair.EntityB);
  auto *p1 = registry.try_get<XPBDParticleComponent>(pair.EntityA);
  auto *p2 = registry.try_get<XPBDParticleComponent>(pair.EntityB);

  if (!t1 || !t2 || !c1 || !c2 || !p1 || !p2)
    return false;

  // Use existing collision detection from PhysicsSystem
  // For now, simple sphere-sphere collision
  if (c1->Type == ColliderType::Sphere && c2->Type == ColliderType::Sphere) {
    const auto &s1 = std::get<SphereCollider>(c1->Shape);
    const auto &s2 = std::get<SphereCollider>(c2->Shape);

    vec3 pos1 = p1->Position + s1.Offset;
    vec3 pos2 = p2->Position + s2.Offset;

    vec3 delta = pos2 - pos1;
    float distSq = Math::LengthSq(delta);
    float radiusSum = s1.Radius + s2.Radius;

    if (distSq > radiusSum * radiusSum)
      return false;

    float dist = std::sqrt(distSq);

    contact.ParticleA = pair.EntityA;
    contact.ParticleB = pair.EntityB;
    contact.Penetration = radiusSum - dist;
    contact.Normal = (dist > 1e-6f) ? (delta / dist) : vec3(0, 1, 0);
    contact.ContactPoint = pos1 + contact.Normal * s1.Radius;
    contact.Friction =
        PhysicsMaterial::CombineDynamicFriction(c1->Friction, c2->Friction);
    contact.Restitution =
        PhysicsMaterial::CombineRestitution(c1->Bounciness, c2->Bounciness);
    contact.Compliance = 0.0f; // Rigid contact

    return true;
  }

  // TODO: Add more collision shapes
  return false;
}

void XPBDSolver::ApplyPositionDelta(XPBDParticleComponent &particle,
                                    const vec3 &delta) {
  if (particle.IsStatic() || particle.IsSleeping)
    return;
  particle.Position += delta;
}

float XPBDSolver::ComputeGeneralizedInverseMass(const XPBDParticleComponent &p1,
                                                const XPBDParticleComponent &p2,
                                                const vec3 &grad1,
                                                const vec3 &grad2) {
  return p1.InverseMass * Math::LengthSq(grad1) +
         p2.InverseMass * Math::LengthSq(grad2);
}

} // namespace Yamen::ECS
