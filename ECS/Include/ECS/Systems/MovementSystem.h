#pragma once

#include "ECS/ISystem.h"
#include "ECS/Scene.h"

namespace Yamen::ECS {

    /**
     * @brief Handles entity movement and pathfinding
     * 
     * TODO: Implement movement logic
     * - Grid-based movement
     * - Pathfinding (A*)
     * - Jumping mechanics
     */
    class MovementSystem : public ISystem {
    public:
        // ISystem interface
        void OnInit(Scene* scene) override {
            // TODO: Initialize navigation grid
        }

        void OnUpdate(Scene* scene, float deltaTime) override {
            // TODO: Process movement requests and update transforms
        }

        void OnRender(Scene* scene) override {
            // Debug rendering for paths
        }

        void OnShutdown(Scene* scene) override {
            // Cleanup
        }

        int GetPriority() const override { return 100; } // Update before physics/animation
        const char* GetName() const override { return "MovementSystem"; }
    };

} // namespace Yamen::ECS
