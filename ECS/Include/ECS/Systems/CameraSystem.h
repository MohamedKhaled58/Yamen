#pragma once

#include "ECS/ISystem.h"
#include "ECS/Scene.h"

namespace Yamen::ECS {

    /**
     * @brief Camera system for managing camera transforms and updates
     * 
     * Features:
     * - Automatic transform synchronization
     * - Aspect ratio management
     * - Camera interpolation
     */
    class CameraSystem : public ISystem {
    public:
        CameraSystem() = default;
        ~CameraSystem() override = default;

        // ISystem interface
        void OnInit(Scene* scene) override;
        void OnUpdate(Scene* scene, float deltaTime) override;
        int GetPriority() const override { return 50; } // Update before rendering
        const char* GetName() const override { return "CameraSystem"; }

        // Aspect ratio management
        void SetViewportSize(uint32_t width, uint32_t height);

    private:
        uint32_t m_ViewportWidth = 1280;
        uint32_t m_ViewportHeight = 720;
    };

} // namespace Yamen::ECS
