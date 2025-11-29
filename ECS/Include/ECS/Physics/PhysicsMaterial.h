#pragma once

#include <memory>
#include <string>

namespace Yamen::ECS {

/**
 * @brief Physics material defining physical properties for collision response
 *
 * Materials control how objects interact during collisions through:
 * - Compliance: Inverse stiffness (how much constraints can stretch)
 * - Friction: Resistance to sliding motion
 * - Restitution: Bounciness of collisions
 * - Damping: Energy dissipation over time
 */
class PhysicsMaterial {
public:
  PhysicsMaterial(const std::string &name = "Default");

  // Material Properties

  /// Compliance: Inverse stiffness (0 = infinitely stiff, higher = softer)
  /// For XPBD: alpha = compliance / (dt^2)
  float Compliance = 0.0f;

  /// Static friction coefficient (0 = frictionless, 1 = high friction)
  float StaticFriction = 0.6f;

  /// Dynamic friction coefficient (typically < static friction)
  float DynamicFriction = 0.4f;

  /// Restitution/bounciness (0 = inelastic, 1 = perfectly elastic)
  float Restitution = 0.3f;

  /// Velocity damping (0 = no damping, 1 = immediate stop)
  float Damping = 0.01f;

  /// Density (kg/m^3) - used for mass calculation
  float Density = 1000.0f; // Water density by default

  /// Material name for debugging
  std::string Name;

  // Combination Rules

  /// Combine two materials to get effective properties for collision
  static PhysicsMaterial Combine(const PhysicsMaterial &a,
                                 const PhysicsMaterial &b);

  /// Get effective static friction between two materials
  static float CombineStaticFriction(float a, float b);

  /// Get effective dynamic friction between two materials
  static float CombineDynamicFriction(float a, float b);

  /// Get effective restitution between two materials
  static float CombineRestitution(float a, float b);

  /// Get effective compliance between two materials
  static float CombineCompliance(float a, float b);

  // Preset Materials
  static std::shared_ptr<PhysicsMaterial> CreateRigid();
  static std::shared_ptr<PhysicsMaterial> CreateRubber();
  static std::shared_ptr<PhysicsMaterial> CreateMetal();
  static std::shared_ptr<PhysicsMaterial> CreateWood();
  static std::shared_ptr<PhysicsMaterial> CreateIce();
  static std::shared_ptr<PhysicsMaterial> CreateCloth();
  static std::shared_ptr<PhysicsMaterial> CreateSoft();
};

} // namespace Yamen::ECS
