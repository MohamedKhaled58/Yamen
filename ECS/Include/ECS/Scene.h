#pragma once

#include <entt/entt.hpp>
#include <Core/Logging/Logger.h>
#include <string>
#include <memory>
#include <vector>

namespace Yamen::ECS {

    class Entity;
    class ISystem;

    /**
     * @brief Scene manages entities, components, and systems
     * 
     * Professional scene management with:
     * - Multi-scene support
     * - System priority execution
     * - Component view caching
     * - Serialization support
     */
    class Scene {
    public:
        Scene(const std::string& name = "Untitled Scene");
        ~Scene();

        // Entity management
        Entity CreateEntity(const std::string& name = "");
        void DestroyEntity(Entity entity);
        
        // System management
        template<typename T, typename... Args>
        T* AddSystem(Args&&... args);
        
        template<typename T>
        T* GetSystem();
        
        template<typename T>
        void RemoveSystem();

        // Lifecycle
        void OnInit();
        void OnUpdate(float deltaTime);
        void OnRender();
        void OnShutdown();

        // Scene state
        bool IsActive() const { return m_Active; }
        void SetActive(bool active) { m_Active = active; }
        
        const std::string& GetName() const { return m_Name; }
        void SetName(const std::string& name) { m_Name = name; }

        // Registry access
        entt::registry& Registry() { return m_Registry; }
        const entt::registry& Registry() const { return m_Registry; }

    private:
        void SortSystems();

        std::string m_Name;
        bool m_Active = true;
        entt::registry m_Registry;
        std::vector<std::unique_ptr<ISystem>> m_Systems;
        bool m_SystemsDirty = false;

        friend class Entity;
    };

    // Template implementations
    template<typename T, typename... Args>
    T* Scene::AddSystem(Args&&... args) {
        static_assert(std::is_base_of<ISystem, T>::value, "T must derive from ISystem");
        
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = system.get();
        m_Systems.push_back(std::move(system));
        m_SystemsDirty = true;
        
        YAMEN_CORE_INFO("Added system: {}", typeid(T).name());
        return ptr;
    }

    template<typename T>
    T* Scene::GetSystem() {
        static_assert(std::is_base_of<ISystem, T>::value, "T must derive from ISystem");
        
        for (auto& system : m_Systems) {
            if (T* ptr = dynamic_cast<T*>(system.get())) {
                return ptr;
            }
        }
        return nullptr;
    }

    template<typename T>
    void Scene::RemoveSystem() {
        static_assert(std::is_base_of<ISystem, T>::value, "T must derive from ISystem");
        
        m_Systems.erase(
            std::remove_if(m_Systems.begin(), m_Systems.end(),
                [](const std::unique_ptr<ISystem>& system) {
                    return dynamic_cast<T*>(system.get()) != nullptr;
                }),
            m_Systems.end()
        );
        m_SystemsDirty = true;
    }

} // namespace Yamen::ECS
