#include "ECS/Systems/GizmoSystem.h"
#include "ECS/Components.h"
#include "ECS/Scene.h"
#include <Core/Logging/Logger.h>
#include <ImGuizmo.h>
#include <imgui.h>

namespace Yamen::ECS {

using namespace Yamen::Core;

void GizmoSystem::OnInit(Scene *scene) {
  YAMEN_CORE_INFO("GizmoSystem initialized");
}

void GizmoSystem::OnUpdate(Scene *scene, float deltaTime) {
  // Keyboard shortcuts are now handled in RenderImGui context
}

void GizmoSystem::HandleKeyboardShortcuts() {
  // Only block when actually typing in a text field
  auto &io = ImGui::GetIO();
  if (io.WantTextInput)
    return;

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

void GizmoSystem::RenderGizmo(Scene *scene) {
  if (!scene)
    return;
  if (m_SelectedEntity == entt::null)
    return;

  auto &registry = scene->Registry();

  // Check if selected entity has transform
  if (!registry.all_of<TransformComponent>(m_SelectedEntity))
    return;

  auto &transform = registry.get<TransformComponent>(m_SelectedEntity);

  // Find primary camera
  auto cameraView = registry.view<CameraComponent, TransformComponent>();
  for (auto camEntity : cameraView) {
    auto [camComp, camTransform] =
        cameraView.get<CameraComponent, TransformComponent>(camEntity);

    if (camComp.Primary) {
      mat4 view = camComp.Camera.GetViewMatrix();
      mat4 projection = camComp.Camera.GetProjectionMatrix();

      // ImGuizmo expects column-major matrices (OpenGL style)
      // DirectXMath is row-major, so we need to transpose
      mat4 viewT = Math::Transpose(view);
      mat4 projT = Math::Transpose(projection);

      mat4 matrix = Math::Translate(transform.Translation) *
                    Math::ToMat4(transform.Rotation) *
                    Math::Scale(mat4(1.0f), transform.Scale);

      mat4 matrixT = Math::Transpose(matrix);

      // Setup ImGuizmo - FIXED: Use background draw list to avoid viewport
      // crash
      ImGuiIO &io = ImGui::GetIO();

      // Set orthographic mode
      ImGuizmo::SetOrthographic(false);

      // CRITICAL: Use background draw list (doesn't require active window)
      ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());

      // Set viewport rect - use full display size
      ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

      // Manipulate
      float snapValues[3] = {m_Snap[0], m_Snap[1], m_Snap[2]};
      ImGuizmo::Manipulate((float *)&viewT, (float *)&projT, m_Operation,
                           m_Mode, (float *)&matrixT, nullptr,
                           m_UseSnap ? snapValues : nullptr);

      // Extract transform if gizmo was used
      if (ImGuizmo::IsUsing()) {
        vec3 translation, rotation, scale;
        float matrixTranslation[3], matrixRotation[3], matrixScale[3];

        ImGuizmo::DecomposeMatrixToComponents(
            (float *)&matrixT, matrixTranslation, matrixRotation, matrixScale);

        translation = vec3(matrixTranslation[0], matrixTranslation[1],
                           matrixTranslation[2]);
        scale = vec3(matrixScale[0], matrixScale[1], matrixScale[2]);

        // Rotation comes in degrees from ImGuizmo
        vec3 rotationRad = vec3(Math::Radians(matrixRotation[0]),
                                Math::Radians(matrixRotation[1]),
                                Math::Radians(matrixRotation[2]));

        // Update transform
        transform.Translation = translation;
        transform.Rotation = quat(rotationRad);
        transform.Scale = scale;

        YAMEN_CORE_TRACE("Gizmo updated: Pos({}, {}, {})", translation.x,
                         translation.y, translation.z);
      }

      break;
    }
  }
}

} // namespace Yamen::ECS
