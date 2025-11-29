#pragma once

#include "ECS/Physics/PhysicsMaterial.h"
#include <Core/Math/Math.h>
#include <entt/entt.hpp>
#include <memory>
#include <variant>
#include <vector>

namespace Yamen::ECS {

using namespace Yamen::Core;

/**
 * @brief XPBD Particle Component
 *
 * Core particle data for XPBD simulation. Each particle represents
 * a point mass that can be constrained to other particles.
 */
struct XPBDParticleComponent {
  vec3 Position = vec3(0.0f);
  vec3 PreviousPosition = vec3(0.0f);
  vec3 Velocity = vec3(0.0f);

  float InverseMass = 1.0f; // 0 = infinite mass (static)

  // External forces accumulator
  vec3 ExternalForce = vec3(0.0f);

  // Sleeping state
  bool IsSleeping = false;
  float SleepTimer = 0.0f;

  // Helper methods
  float GetMass() const {
    return InverseMass > 0.0f ? 1.0f / InverseMass : 0.0f;
  }
  void SetMass(float mass) { InverseMass = mass > 0.0f ? 1.0f / mass : 0.0f; }
  bool IsStatic() const { return InverseMass == 0.0f; }

  void AddForce(const vec3 &force) { ExternalForce += force; }
};

/**
 * @brief Base constraint data for XPBD
 *
 * All constraints store their Lagrange multiplier for warm-starting
 * and compliance for controlling stiffness.
 */
struct XPBDConstraintBase {
  float Compliance = 0.0f; // Inverse stiffness (0 = rigid)
  float Lambda = 0.0f;     // Lagrange multiplier (for warm starting)
  bool Active = true;

  virtual ~XPBDConstraintBase() = default;
};

/**
 * @brief Distance Constraint
 *
 * Maintains a fixed or spring-like distance between two particles.
 * C = |p1 - p2| - rest_length
 */
struct DistanceConstraint : public XPBDConstraintBase {
  entt::entity ParticleA = entt::null;
  entt::entity ParticleB = entt::null;
  float RestLength = 1.0f;

  // Optional: Set to true for inequality constraint (rope, not rod)
  bool IsRope = false;

  DistanceConstraint() = default;
  DistanceConstraint(entt::entity a, entt::entity b, float length,
                     float compliance = 0.0f)
      : ParticleA(a), ParticleB(b), RestLength(length) {
    Compliance = compliance;
  }
};

/**
 * @brief Contact Constraint
 *
 * Prevents penetration between colliding objects.
 * C = dot(p1 - p2, normal) - penetration
 */
struct ContactConstraint : public XPBDConstraintBase {
  entt::entity ParticleA = entt::null;
  entt::entity ParticleB = entt::null;
  vec3 Normal = vec3(0, 1, 0);
  float Penetration = 0.0f;

  // Contact point in world space
  vec3 ContactPoint = vec3(0.0f);

  // Material properties for this contact
  float Friction = 0.5f;
  float Restitution = 0.3f;

  ContactConstraint() = default;
  ContactConstraint(entt::entity a, entt::entity b, const vec3 &normal,
                    float penetration)
      : ParticleA(a), ParticleB(b), Normal(normal), Penetration(penetration) {}
};

/**
 * @brief Bending Constraint
 *
 * Controls the angle between two edges (for cloth simulation).
 * C = acos(dot(n1, n2)) - rest_angle
 */
struct BendingConstraint : public XPBDConstraintBase {
  entt::entity Particle0 = entt::null; // Shared edge vertex 1
  entt::entity Particle1 = entt::null; // Shared edge vertex 2
  entt::entity Particle2 = entt::null; // Triangle 1 opposite vertex
  entt::entity Particle3 = entt::null; // Triangle 2 opposite vertex

  float RestAngle = 0.0f; // Angle in radians

  BendingConstraint() = default;
  BendingConstraint(entt::entity p0, entt::entity p1, entt::entity p2,
                    entt::entity p3, float restAngle, float compliance = 0.01f)
      : Particle0(p0), Particle1(p1), Particle2(p2), Particle3(p3),
        RestAngle(restAngle) {
    Compliance = compliance;
  }
};

/**
 * @brief Volume Constraint
 *
 * Preserves volume of a tetrahedral element (for soft bodies).
 * C = V_current - V_rest
 */
struct VolumeConstraint : public XPBDConstraintBase {
  entt::entity Particle0 = entt::null;
  entt::entity Particle1 = entt::null;
  entt::entity Particle2 = entt::null;
  entt::entity Particle3 = entt::null;

  float RestVolume = 1.0f;

  VolumeConstraint() = default;
  VolumeConstraint(entt::entity p0, entt::entity p1, entt::entity p2,
                   entt::entity p3, float restVolume, float compliance = 0.0f)
      : Particle0(p0), Particle1(p1), Particle2(p2), Particle3(p3),
        RestVolume(restVolume) {
    Compliance = compliance;
  }
};

/**
 * @brief Shape Matching Constraint
 *
 * Matches current particle configuration to rest configuration.
 * Used for rigid body simulation and soft body stiffness.
 */
struct ShapeMatchingConstraint : public XPBDConstraintBase {
  std::vector<entt::entity> Particles;
  std::vector<vec3> RestPositions; // Relative to center of mass
  vec3 RestCenterOfMass = vec3(0.0f);

  // Allow rotation and scaling
  bool AllowRotation = true;
  bool AllowScaling = false;

  ShapeMatchingConstraint() = default;
};

/**
 * @brief Ball-Socket Joint Constraint
 *
 * Constrains two particles to be at the same position.
 * C = p1 - p2
 */
struct BallSocketConstraint : public XPBDConstraintBase {
  entt::entity ParticleA = entt::null;
  entt::entity ParticleB = entt::null;

  // Local attachment points (relative to particle position)
  vec3 LocalAnchorA = vec3(0.0f);
  vec3 LocalAnchorB = vec3(0.0f);

  BallSocketConstraint() = default;
  BallSocketConstraint(entt::entity a, entt::entity b, float compliance = 0.0f)
      : ParticleA(a), ParticleB(b) {
    Compliance = compliance;
  }
};

/**
 * @brief Hinge Joint Constraint
 *
 * Constrains rotation to a single axis.
 */
struct HingeConstraint : public XPBDConstraintBase {
  entt::entity ParticleA = entt::null;
  entt::entity ParticleB = entt::null;

  vec3 LocalAnchorA = vec3(0.0f);
  vec3 LocalAnchorB = vec3(0.0f);
  vec3 HingeAxis = vec3(0, 1, 0);

  // Limits
  bool UseLimits = false;
  float MinAngle = -Math::PI;
  float MaxAngle = Math::PI;

  // Motor
  bool UseMotor = false;
  float TargetVelocity = 0.0f;
  float MaxMotorForce = 100.0f;

  HingeConstraint() = default;
};

/**
 * @brief Slider Joint Constraint
 *
 * Constrains motion to a single axis (prismatic joint).
 */
struct SliderConstraint : public XPBDConstraintBase {
  entt::entity ParticleA = entt::null;
  entt::entity ParticleB = entt::null;

  vec3 SlideAxis = vec3(1, 0, 0);

  // Limits
  bool UseLimits = false;
  float MinDistance = -10.0f;
  float MaxDistance = 10.0f;

  SliderConstraint() = default;
};

/**
 * @brief XPBD Constraint Component
 *
 * Variant holding any constraint type. Attached to entities
 * to participate in constraint solving.
 */
struct XPBDConstraintComponent {
  using ConstraintVariant =
      std::variant<DistanceConstraint, ContactConstraint, BendingConstraint,
                   VolumeConstraint, ShapeMatchingConstraint,
                   BallSocketConstraint, HingeConstraint, SliderConstraint>;

  ConstraintVariant Constraint;

  // Priority for solving order (higher = solved first)
  int Priority = 0;

  // Material override (if null, uses default)
  std::shared_ptr<PhysicsMaterial> Material;

  // Constructors for convenience
  XPBDConstraintComponent() = default;

  template <typename T>
  XPBDConstraintComponent(const T &constraint) : Constraint(constraint) {}

  // Helper to get constraint base
  XPBDConstraintBase *GetBase() {
    return std::visit([](auto &c) -> XPBDConstraintBase * { return &c; },
                      Constraint);
  }

  const XPBDConstraintBase *GetBase() const {
    return std::visit(
        [](const auto &c) -> const XPBDConstraintBase * { return &c; },
        Constraint);
  }
};

} // namespace Yamen::ECS
