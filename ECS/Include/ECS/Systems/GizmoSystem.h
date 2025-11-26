#pragma once

#include "ECS/ISystem.h"
#include <imgui.h>
#include <ImGuizmo.h>
#include <glm/glm.hpp>
#include <entt/entt.hpp>

namespace Yamen::ECS {

    /**
     * @brief Gizmo System - Professional transform manipulation
     * 
     * Provides Unity/Unreal-style transform gizmos for entity manipulation.
     * Can be added to any scene that needs visual transform editing.
     */
    class GizmoSystem : public ISystem {
    public:
        GizmoSystem() = default;
        ~GizmoSystem() = default;

        void OnInit(Scene* scene) override;
        void OnUpdate(Scene* scene, float deltaTime) override;
        void OnRender(Scene* scene) override {}
        void OnShutdown(Scene* scene) override {}
        const char* GetName() const override { return "GizmoSystem"; }

        // Gizmo control
        void SetSelectedEntity(entt::entity entity) { m_SelectedEntity = entity; }
        entt::entity GetSelectedEntity() const { return m_SelectedEntity; }
        
        void SetOperation(ImGuizmo::OPERATION op) { m_Operation = op; }
        ImGuizmo::OPERATION GetOperation() const { return m_Operation; }
        
        void SetMode(ImGuizmo::MODE mode) { m_Mode = mode; }
        ImGuizmo::MODE GetMode() const { return m_Mode; }
        
        void SetSnapEnabled(bool enabled) { m_UseSnap = enabled; }
        bool IsSnapEnabled() const { return m_UseSnap; }
        
        void SetSnapValues(float x, float y, float z) {
            m_Snap[0] = x;
            m_Snap[1] = y;
            m_Snap[2] = z;
        }

        // Render gizmo (call from ImGui context)
        void RenderGizmo(Scene* scene);

        // Handle keyboard shortcuts
        void HandleKeyboardShortcuts();

    private:
        entt::entity m_SelectedEntity = entt::null;
        ImGuizmo::OPERATION m_Operation = ImGuizmo::TRANSLATE;
        ImGuizmo::MODE m_Mode = ImGuizmo::WORLD;
        bool m_UseSnap = false;
        float m_Snap[3] = { 1.0f, 1.0f, 1.0f };
    };

} // namespace Yamen::ECS
