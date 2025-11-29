#include "ECS/Physics/PhysicsMaterial.h"
#include <algorithm>
#include <cmath>


namespace Yamen::ECS {

PhysicsMaterial::PhysicsMaterial(const std::string &name) : Name(name) {}

PhysicsMaterial PhysicsMaterial::Combine(const PhysicsMaterial &a,
                                         const PhysicsMaterial &b) {
  PhysicsMaterial result("Combined");
  result.StaticFriction =
      CombineStaticFriction(a.StaticFriction, b.StaticFriction);
  result.DynamicFriction =
      CombineDynamicFriction(a.DynamicFriction, b.DynamicFriction);
  result.Restitution = CombineRestitution(a.Restitution, b.Restitution);
  result.Compliance = CombineCompliance(a.Compliance, b.Compliance);
  result.Damping = (a.Damping + b.Damping) * 0.5f;
  return result;
}

float PhysicsMaterial::CombineStaticFriction(float a, float b) {
  // Geometric mean (common in physics engines)
  return std::sqrt(a * b);
}

float PhysicsMaterial::CombineDynamicFriction(float a, float b) {
  // Geometric mean
  return std::sqrt(a * b);
}

float PhysicsMaterial::CombineRestitution(float a, float b) {
  // Maximum restitution (most bouncy wins)
  return std::max(a, b);
}

float PhysicsMaterial::CombineCompliance(float a, float b) {
  // Series combination (like springs in series)
  // 1/k_eff = 1/k_a + 1/k_b
  // Since compliance = 1/k: c_eff = c_a + c_b
  return a + b;
}

std::shared_ptr<PhysicsMaterial> PhysicsMaterial::CreateRigid() {
  auto mat = std::make_shared<PhysicsMaterial>("Rigid");
  mat->Compliance = 0.0f;
  mat->StaticFriction = 0.6f;
  mat->DynamicFriction = 0.4f;
  mat->Restitution = 0.3f;
  mat->Damping = 0.01f;
  mat->Density = 2000.0f; // Concrete-like
  return mat;
}

std::shared_ptr<PhysicsMaterial> PhysicsMaterial::CreateRubber() {
  auto mat = std::make_shared<PhysicsMaterial>("Rubber");
  mat->Compliance = 0.001f;
  mat->StaticFriction = 1.0f;
  mat->DynamicFriction = 0.8f;
  mat->Restitution = 0.9f;
  mat->Damping = 0.05f;
  mat->Density = 1100.0f;
  return mat;
}

std::shared_ptr<PhysicsMaterial> PhysicsMaterial::CreateMetal() {
  auto mat = std::make_shared<PhysicsMaterial>("Metal");
  mat->Compliance = 0.0f;
  mat->StaticFriction = 0.4f;
  mat->DynamicFriction = 0.3f;
  mat->Restitution = 0.5f;
  mat->Damping = 0.005f;
  mat->Density = 7800.0f; // Steel
  return mat;
}

std::shared_ptr<PhysicsMaterial> PhysicsMaterial::CreateWood() {
  auto mat = std::make_shared<PhysicsMaterial>("Wood");
  mat->Compliance = 0.0001f;
  mat->StaticFriction = 0.5f;
  mat->DynamicFriction = 0.3f;
  mat->Restitution = 0.4f;
  mat->Damping = 0.02f;
  mat->Density = 600.0f;
  return mat;
}

std::shared_ptr<PhysicsMaterial> PhysicsMaterial::CreateIce() {
  auto mat = std::make_shared<PhysicsMaterial>("Ice");
  mat->Compliance = 0.0f;
  mat->StaticFriction = 0.1f;
  mat->DynamicFriction = 0.05f;
  mat->Restitution = 0.2f;
  mat->Damping = 0.001f;
  mat->Density = 917.0f;
  return mat;
}

std::shared_ptr<PhysicsMaterial> PhysicsMaterial::CreateCloth() {
  auto mat = std::make_shared<PhysicsMaterial>("Cloth");
  mat->Compliance = 0.01f;
  mat->StaticFriction = 0.7f;
  mat->DynamicFriction = 0.5f;
  mat->Restitution = 0.1f;
  mat->Damping = 0.1f;
  mat->Density = 200.0f;
  return mat;
}

std::shared_ptr<PhysicsMaterial> PhysicsMaterial::CreateSoft() {
  auto mat = std::make_shared<PhysicsMaterial>("Soft");
  mat->Compliance = 0.1f;
  mat->StaticFriction = 0.8f;
  mat->DynamicFriction = 0.6f;
  mat->Restitution = 0.2f;
  mat->Damping = 0.2f;
  mat->Density = 500.0f;
  return mat;
}

} // namespace Yamen::ECS
