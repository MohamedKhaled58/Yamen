#pragma once

#include "ECS/ISystem.h"
#include "ECS/Scene.h"

namespace Yamen::ECS {

    /**
     * @brief Script system for managing native C++ scripts
     * 
     * Features:
     * - Automatic script instantiation
     * - Lifecycle management (OnCreate, OnUpdate, OnDestroy)
     * - Hot reload support (future)
     */
    class ScriptSystem : public ISystem {
    public:
        ScriptSystem() = default;
        ~ScriptSystem() override = default;

        // ISystem interface
        void OnInit(Scene* scene) override;
        void OnUpdate(Scene* scene, float deltaTime) override;
        void OnShutdown(Scene* scene) override;
        int GetPriority() const override { return 100; } // Update after camera, before rendering
        const char* GetName() const override { return "ScriptSystem"; }
    };

} // namespace Yamen::ECS
