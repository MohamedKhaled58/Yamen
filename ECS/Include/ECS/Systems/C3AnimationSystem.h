#pragma once

#include "ECS/ISystem.h"
#include "ECS/Scene.h"

namespace Yamen::ECS {

    /**
     * @brief Handles C3 model animation playback
     * 
     * TODO: Implement C3 animation logic
     * - Bone transformation updates
     * - Animation state machine
     * - Blending
     */
    class C3AnimationSystem : public ISystem {
    public:
        // ISystem interface
        void OnInit(Scene* scene) override {
            // TODO: Initialize animation resources
        }

        void OnUpdate(Scene* scene, float deltaTime) override {
            // TODO: Update animation states and bone matrices
        }

        void OnRender(Scene* scene) override {
            // Debug rendering for skeletons could go here
        }

        void OnShutdown(Scene* scene) override {
            // Cleanup
        }

        int GetPriority() const override { return 200; } // Update before rendering
        const char* GetName() const override { return "C3AnimationSystem"; }
    };

} // namespace Yamen::ECS
