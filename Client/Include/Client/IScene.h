#pragma once

#include <string>

namespace Yamen {

    /**
     * @brief Base interface for all scenes in the engine
     * 
     * Provides a common lifecycle interface that all scenes must implement.
     * This enables the SceneManager to handle scenes polymorphically.
     */
    class IScene {
    public:
        virtual ~IScene() = default;

        /**
         * @brief Initialize scene resources
         * @return true if initialization succeeded
         */
        virtual bool Initialize() = 0;

        /**
         * @brief Update scene logic
         * @param deltaTime Time since last frame in seconds
         */
        virtual void Update(float deltaTime) = 0;

        /**
         * @brief Render scene geometry
         */
        virtual void Render() = 0;

        /**
         * @brief Render scene ImGui UI
         */
        virtual void RenderImGui() = 0;

        /**
         * @brief Get scene name for identification
         */
        virtual const char* GetName() const = 0;

        /**
         * @brief Called when scene becomes active
         */
        virtual void OnActivate() {}

        /**
         * @brief Called when scene becomes inactive
         */
        virtual void OnDeactivate() {}
    };

} // namespace Yamen
