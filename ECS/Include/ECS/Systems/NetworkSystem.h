#pragma once

#include "ECS/ISystem.h"
#include "ECS/Scene.h"

namespace Yamen::ECS {

    /**
     * @brief Handles network state synchronization
     * 
     * TODO: Implement network logic
     * - Entity spawning/despawning from packets
     * - Position interpolation
     * - State synchronization
     */
    class NetworkSystem : public ISystem {
    public:
        // ISystem interface
        void OnInit(Scene* scene) override {
            // TODO: Connect to network manager
        }

        void OnUpdate(Scene* scene, float deltaTime) override {
            // TODO: Process incoming packets and sync entities
        }

        void OnRender(Scene* scene) override {
            // Network debug info
        }

        void OnShutdown(Scene* scene) override {
            // Cleanup
        }

        int GetPriority() const override { return 50; } // Update early
        const char* GetName() const override { return "NetworkSystem"; }
    };

} // namespace Yamen::ECS
