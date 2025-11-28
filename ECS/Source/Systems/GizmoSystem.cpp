#include "ECS/Systems/GizmoSystem.h"
#include "ECS/Components.h"
#include "ECS/Scene.h"
#include <Core/Logging/Logger.h>
#include <imgui.h>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Yamen::ECS {

    void GizmoSystem::OnInit(Scene* scene) {
        YAMEN_CORE_INFO("GizmoSystem initialized");
    }

    void GizmoSystem::OnUpdate(Scene* scene, float deltaTime) {
        // Keyboard shortcuts are now handled in RenderImGui context
    }

    void GizmoSystem::HandleKeyboardShortcuts() {
        // Only block when actually typing in a text field
        auto& io = ImGui::GetIO();
        if (io.WantTextInput) return;
        
        // W/E/R shortcuts (Unity-style)
        if (ImGui::IsKeyPressed(ImGuiKey_W, false)) {
            m_Operation = ImGuizmo::TRANSLATE;
            YAMEN_CORE_INFO("Gizmo: Translate mode");
        }
        if (ImGui::IsKeyPressed(ImGuiKey_E, false)) {
            m_Operation = ImGuizmo::ROTATE;
            YAMEN_CORE_INFO("Gizmo: Rotate mode");
        }
        if (ImGui::IsKeyPressed(ImGuiKey_R, false)) {
            m_Operation = ImGuizmo::SCALE;
            YAMEN_CORE_INFO("Gizmo: Scale mode");
        }
    }

    void GizmoSystem::RenderGizmo(Scene* scene) {
        if (!scene) return;
        if (m_SelectedEntity == entt::null) return;

        auto& registry = scene->Registry();
        
        // Check if selected entity has transform
        if (!registry.all_of<TransformComponent>(m_SelectedEntity)) return;

        auto& transform = registry.get<TransformComponent>(m_SelectedEntity);

        // Find primary camera
        auto cameraView = registry.view<CameraComponent, TransformComponent>();
        for (auto camEntity : cameraView) {
            auto [camComp, camTransform] = cameraView.get<CameraComponent, TransformComponent>(camEntity);
            
            if (camComp.Primary) {
                glm::mat4 view = camComp.Camera.GetViewMatrix();
                glm::mat4 projection = camComp.Camera.GetProjectionMatrix();
                
                glm::mat4 matrix = glm::translate(glm::mat4(1.0f), transform.Translation);
                matrix *= glm::mat4_cast(transform.Rotation);
                matrix = glm::scale(matrix, transform.Scale);
                
                // Setup ImGuizmo - FIXED: Use background draw list to avoid viewport crash
                ImGuiIO& io = ImGui::GetIO();
                
                // Set orthographic mode
                ImGuizmo::SetOrthographic(false);
                
                // CRITICAL: Use background draw list (doesn't require active window)
                ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
                
                // Set viewport rect - use full display size
                ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
                
                // Manipulate
                float snapValues[3] = { m_Snap[0], m_Snap[1], m_Snap[2] };
                ImGuizmo::Manipulate(
                    glm::value_ptr(view),
                    glm::value_ptr(projection),
                    m_Operation,
                    m_Mode,
                    glm::value_ptr(matrix),
                    nullptr,
                    m_UseSnap ? snapValues : nullptr
                );
                
                // Extract transform if gizmo was used
                if (ImGuizmo::IsUsing()) {
                    glm::vec3 translation, rotation, scale;
                    ImGuizmo::DecomposeMatrixToComponents(
                        glm::value_ptr(matrix),
                        glm::value_ptr(translation),
                        glm::value_ptr(rotation),
                        glm::value_ptr(scale)
                    );
                    
                    // Update transform - rotation comes from ImGuizmo in degrees
                    transform.Translation = translation;
                    transform.Rotation = glm::quat(glm::radians(rotation));
                    transform.Scale = scale;
                    
                    YAMEN_CORE_TRACE("Gizmo updated: Pos({}, {}, {})", 
                        translation.x, translation.y, translation.z);
                }
                
                break;
            }
        }
    }

} // namespace Yamen::ECS
