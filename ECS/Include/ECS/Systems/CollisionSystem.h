#pragma once

#include "ECS/ISystem.h"
#include "ECS/Scene.h"

namespace Yamen::ECS {

    /**
     * @brief Handles collision detection
     * 
     * TODO: Implement collision logic
     * - AABB/OBB intersection
     * - Raycasting
     * - Trigger events
     */
    class CollisionSystem : public ISystem {
    public:
        // ISystem interface
        void OnInit(Scene* scene) override {
            // TODO: Initialize collision structures
        }

        void OnUpdate(Scene* scene, float deltaTime) override {
            // TODO: Check for collisions and resolve
        }

        void OnRender(Scene* scene) override {
            // Debug rendering for collision shapes
        }

        void OnShutdown(Scene* scene) override {
            // Cleanup
        }

        int GetPriority() const override { return 160; } // Update after physics
        const char* GetName() const override { return "CollisionSystem"; }
    };

} // namespace Yamen::ECS
