#pragma once

#include <Core/Logging/Logger.h>

namespace Yamen::ECS {

    class Scene;

    /**
     * @brief Base interface for all ECS systems
     * 
     * Systems process entities with specific component combinations.
     * Priority determines execution order (lower = earlier).
     */
    class ISystem {
    public:
        virtual ~ISystem() = default;

        // Lifecycle hooks
        virtual void OnInit(Scene* scene) {}
        virtual void OnUpdate(Scene* scene, float deltaTime) {}
        virtual void OnRender(Scene* scene) {}
        virtual void OnShutdown(Scene* scene) {}

        // Priority for execution order (lower = earlier)
        virtual int GetPriority() const { return 100; }
        
        // System name for debugging
        virtual const char* GetName() const = 0;
    };

} // namespace Yamen::ECS
