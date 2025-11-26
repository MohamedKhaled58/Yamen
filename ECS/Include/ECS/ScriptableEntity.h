#pragma once

#include "ECS/Entity.h"

namespace Yamen::ECS {

    /**
     * @brief Base class for native C++ scripts
     * 
     * Derive from this class to create custom gameplay logic.
     * Override OnCreate, OnUpdate, and OnDestroy for lifecycle hooks.
     * 
     * Example:
     * ```cpp
     * class PlayerController : public ScriptableEntity {
     * public:
     *     void OnCreate() override {
     *         // Initialize
     *     }
     *     
     *     void OnUpdate(float deltaTime) override {
     *         auto& transform = GetComponent<TransformComponent>();
     *         // Update logic
     *     }
     *     
     *     void OnDestroy() override {
     *         // Cleanup
     *     }
     * };
     * ```
     */
    class ScriptableEntity {
    public:
        virtual ~ScriptableEntity() = default;

        template<typename T>
        T& GetComponent() {
            return m_Entity.GetComponent<T>();
        }

        template<typename T>
        bool HasComponent() {
            return m_Entity.HasComponent<T>();
        }

        template<typename T, typename... Args>
        T& AddComponent(Args&&... args) {
            return m_Entity.AddComponent<T>(std::forward<Args>(args)...);
        }

        template<typename T>
        void RemoveComponent() {
            m_Entity.RemoveComponent<T>();
        }

        virtual void OnCreate() {}
        virtual void OnUpdate(float deltaTime) {}
        virtual void OnDestroy() {}

    private:
        Entity m_Entity;
        friend class ScriptSystem;
    };

} // namespace Yamen::ECS
