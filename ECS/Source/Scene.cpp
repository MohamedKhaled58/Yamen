#include "ECS/Scene.h"
#include "ECS/Entity.h"
#include "ECS/Components.h"
#include "ECS/ISystem.h"
#include <Core/Logging/Logger.h>
#include <algorithm>

namespace Yamen::ECS {

    Scene::Scene(const std::string& name)
        : m_Name(name)
        , m_Active(true)
        , m_SystemsDirty(false)
    {
        YAMEN_CORE_INFO("Created scene: {}", m_Name);
    }

    Scene::~Scene() {
        OnShutdown();
        YAMEN_CORE_INFO("Destroyed scene: {}", m_Name);
    }

    Entity Scene::CreateEntity(const std::string& name) {
        Entity entity = { m_Registry.create(), this };
        
        // Add default components
        auto& tag = entity.AddComponent<TagComponent>();
        tag.Tag = name.empty() ? "Entity" : name;
        
        entity.AddComponent<TransformComponent>();
        
        return entity;
    }

    void Scene::DestroyEntity(Entity entity) {
        if (m_Registry.valid(entity.m_EntityHandle)) {
            m_Registry.destroy(entity.m_EntityHandle);
        }
    }

    void Scene::OnInit() {
        if (m_SystemsDirty) {
            SortSystems();
        }

        for (auto& system : m_Systems) {
            system->OnInit(this);
        }
    }

    void Scene::OnUpdate(float deltaTime) {
        if (!m_Active) return;

        if (m_SystemsDirty) {
            SortSystems();
        }

        for (auto& system : m_Systems) {
            system->OnUpdate(this, deltaTime);
        }
    }

    void Scene::OnRender() {
        if (!m_Active) return;

        if (m_SystemsDirty) {
            SortSystems();
        }

        for (auto& system : m_Systems) {
            system->OnRender(this);
        }
    }

    void Scene::OnShutdown() {
        for (auto& system : m_Systems) {
            system->OnShutdown(this);
        }
        m_Systems.clear();
        m_Registry.clear();
    }

    void Scene::SortSystems() {
        std::sort(m_Systems.begin(), m_Systems.end(),
            [](const std::unique_ptr<ISystem>& a, const std::unique_ptr<ISystem>& b) {
                return a->GetPriority() < b->GetPriority();
            });
        m_SystemsDirty = false;
    }

} // namespace Yamen::ECS
