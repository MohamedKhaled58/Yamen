#pragma once

#include <Core/Math/Math.h>
#include <entt/entt.hpp>
#include <string>
#include <vector>


namespace Yamen::ECS {

using namespace Yamen::Core;

/**
 * @brief Tag component for entity identification
 */
struct TagComponent {
  std::string Tag;

  TagComponent() = default;
  TagComponent(const TagComponent &) = default;
  TagComponent(const std::string &tag) : Tag(tag) {}
};

/**
 * @brief Transform component for position, rotation, and scale
 *
 * Uses quaternions for rotation to avoid gimbal lock.
 * Provides cached transform matrix for performance.
 */
struct TransformComponent {
  vec3 Translation = vec3(0.0f);
  quat Rotation = quat(0.0f, 0.0f, 0.0f, 1.0f); // Identity quaternion
  vec3 Scale = vec3(1.0f);

  TransformComponent() = default;
  TransformComponent(const TransformComponent &) = default;
  TransformComponent(const vec3 &translation) : Translation(translation) {}

  mat4 GetTransform() const {
    return Math::Translate(Translation) * Math::ToMat4(Rotation) *
           Math::Scale(mat4(1.0f), Scale);
  }

  // Euler angle helpers (in radians)
  void SetRotationEuler(const vec3 &euler) { Rotation = quat(euler); }

  vec3 GetRotationEuler() const { return Math::ToEulerAngles(Rotation); }

  // Direction vectors
  vec3 GetForward() const {
    return Math::Normalize(Math::Rotate(Rotation, vec3(0.0f, 0.0f, 1.0f)));
  }

  vec3 GetRight() const {
    return Math::Normalize(Math::Rotate(Rotation, vec3(1.0f, 0.0f, 0.0f)));
  }

  vec3 GetUp() const {
    return Math::Normalize(Math::Rotate(Rotation, vec3(0.0f, 1.0f, 0.0f)));
  }
};

/**
 * @brief Hierarchy component for parent-child relationships
 */
struct HierarchyComponent {
  entt::entity Parent = entt::null;
  std::vector<entt::entity> Children;

  HierarchyComponent() = default;
  HierarchyComponent(const HierarchyComponent &) = default;
};

} // namespace Yamen::ECS
