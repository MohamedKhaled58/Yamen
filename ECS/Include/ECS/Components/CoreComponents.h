#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <string>
#include <vector>
#include <entt/entt.hpp>

namespace Yamen::ECS {

    /**
     * @brief Tag component for entity identification
     */
    struct TagComponent {
        std::string Tag;

        TagComponent() = default;
        TagComponent(const TagComponent&) = default;
        TagComponent(const std::string& tag) : Tag(tag) {}
    };

    /**
     * @brief Transform component for position, rotation, and scale
     * 
     * Uses quaternions for rotation to avoid gimbal lock.
     * Provides cached transform matrix for performance.
     */
    struct TransformComponent {
        glm::vec3 Translation = glm::vec3(0.0f);
        glm::quat Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion
        glm::vec3 Scale = glm::vec3(1.0f);

        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;
        TransformComponent(const glm::vec3& translation) : Translation(translation) {}

        glm::mat4 GetTransform() const {
            return glm::translate(glm::mat4(1.0f), Translation)
                 * glm::toMat4(Rotation)
                 * glm::scale(glm::mat4(1.0f), Scale);
        }

        // Euler angle helpers (in radians)
        void SetRotationEuler(const glm::vec3& euler) {
            Rotation = glm::quat(euler);
        }

        glm::vec3 GetRotationEuler() const {
            return glm::eulerAngles(Rotation);
        }

        // Direction vectors
        glm::vec3 GetForward() const {
            return glm::normalize(Rotation * glm::vec3(0.0f, 0.0f, 1.0f));
        }

        glm::vec3 GetRight() const {
            return glm::normalize(Rotation * glm::vec3(1.0f, 0.0f, 0.0f));
        }

        glm::vec3 GetUp() const {
            return glm::normalize(Rotation * glm::vec3(0.0f, 1.0f, 0.0f));
        }
    };

    /**
     * @brief Hierarchy component for parent-child relationships
     */
    struct HierarchyComponent {
        entt::entity Parent = entt::null;
        std::vector<entt::entity> Children;

        HierarchyComponent() = default;
        HierarchyComponent(const HierarchyComponent&) = default;
    };

} // namespace Yamen::ECS
