#pragma once

#include "ECS/Scene.h"
#include <entt/entt.hpp>
#include <Core/Logging/Logger.h>

namespace Yamen::ECS {

    class Entity {
    public:
        Entity() = default;
        Entity(entt::entity handle, Scene* scene)
            : m_EntityHandle(handle), m_Scene(scene) {}
        Entity(const Entity& other) = default;

        template<typename T, typename... Args>
        T& AddComponent(Args&&... args) {
            if (HasComponent<T>()) {
                YAMEN_CORE_WARN("Entity already has component!");
                return GetComponent<T>();
            }
            return m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
        }

        template<typename T>
        T& GetComponent() {
            if (!HasComponent<T>()) {
                YAMEN_CORE_ERROR("Entity does not have component!");
                throw std::runtime_error("Entity does not have component!");
            }
            return m_Scene->m_Registry.get<T>(m_EntityHandle);
        }

        template<typename T>
        bool HasComponent() {
            return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
        }

        template<typename T>
        void RemoveComponent() {
            if (!HasComponent<T>()) {
                YAMEN_CORE_WARN("Entity does not have component to remove!");
                return;
            }
            m_Scene->m_Registry.remove<T>(m_EntityHandle);
        }

        operator bool() const { return m_EntityHandle != entt::null; }
        operator entt::entity() const { return m_EntityHandle; }
        
        bool operator==(const Entity& other) const {
            return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
        }

        bool operator!=(const Entity& other) const {
            return !(*this == other);
        }

        entt::entity m_EntityHandle{ entt::null };
        Scene* m_Scene = nullptr;

    private:
        friend class Scene;
        friend class ScriptSystem;
    };

} // namespace Yamen::ECS
