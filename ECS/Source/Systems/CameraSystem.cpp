#include "ECS/Systems/CameraSystem.h"
#include "ECS/Components.h"
#include <Core/Logging/Logger.h>
#include <entt/entt.hpp>

namespace Yamen::ECS {

    void CameraSystem::OnInit(Scene* scene) {
        YAMEN_CORE_INFO("CameraSystem initialized");
    }

    void CameraSystem::OnUpdate(Scene* scene, float deltaTime) {
        if (!scene) return;

        auto view = scene->Registry().view<TransformComponent, CameraComponent>();
        
        for (auto entity : view) {
            auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);
            
            // Sync camera position and rotation with transform
            camera.Camera.SetTransform(transform.Translation, transform.Rotation);
            
            // Update aspect ratio if not fixed
            if (!camera.FixedAspectRatio && m_ViewportWidth > 0 && m_ViewportHeight > 0) {
                float aspectRatio = static_cast<float>(m_ViewportWidth) / static_cast<float>(m_ViewportHeight);
                camera.Camera.SetAspectRatio(aspectRatio);
            }
        }
    }

    void CameraSystem::SetViewportSize(uint32_t width, uint32_t height) {
        m_ViewportWidth = width;
        m_ViewportHeight = height;
    }

} // namespace Yamen::ECS
