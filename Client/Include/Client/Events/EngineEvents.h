#pragma once

#include "Platform/Events/Event.h"
#include <string>
#include <entt/entt.hpp>

namespace Yamen::Client {

    // Define a custom category for Engine events if not already present in Platform
    // For now we can use EventCategory::Application or EventCategory::Custom

    /**
     * @brief Event fired when the active scene changes
     */
    class SceneChangedEvent : public Platform::Event {
    public:
        SceneChangedEvent(const std::string& sceneName)
            : m_SceneName(sceneName) {}

        Platform::EventCategory GetCategory() const override { return Platform::EventCategory::Application; }
        const char* GetName() const override { return "SceneChangedEvent"; }
        std::string ToString() const override { return "SceneChangedEvent: " + m_SceneName; }

        const std::string& GetSceneName() const { return m_SceneName; }

    private:
        std::string m_SceneName;
    };

    /**
     * @brief Event fired when an entity is selected in the editor/scene
     */
    class EntitySelectedEvent : public Platform::Event {
    public:
        EntitySelectedEvent(entt::entity entity)
            : m_Entity(entity) {}

        Platform::EventCategory GetCategory() const override { return Platform::EventCategory::Custom; }
        const char* GetName() const override { return "EntitySelectedEvent"; }
        std::string ToString() const override { return "EntitySelectedEvent: " + std::to_string((uint32_t)m_Entity); }

        entt::entity GetEntity() const { return m_Entity; }

    private:
        entt::entity m_Entity;
    };

} // namespace Yamen::Client
