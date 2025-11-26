#pragma once

#include "Client/IScene.h"
#include "Graphics/RHI/GraphicsDevice.h"
#include <Core/Logging/Logger.h>
#include <memory>
#include <unordered_map>
#include <string>

namespace Yamen::Client {

    /**
     * @brief Manages scene lifecycle and transitions
     * 
     * Features:
     * - Dynamic scene registration
     * - Scene switching with activate/deactivate hooks
     * - Scene factory pattern for lazy initialization
     */
    class SceneManager {
    public:
        using SceneFactory = std::function<std::unique_ptr<IScene>()>;

        SceneManager(Graphics::GraphicsDevice& device);
        ~SceneManager();

        /**
         * @brief Register a scene factory
         * @param name Scene identifier
         * @param factory Function that creates the scene
         */
        void RegisterScene(const std::string& name, SceneFactory factory);

        /**
         * @brief Load and activate a scene by name
         * @param name Scene identifier
         * @return true if scene was loaded successfully
         */
        bool LoadScene(const std::string& name);

        /**
         * @brief Get the currently active scene
         */
        IScene* GetActiveScene() const { return m_ActiveScene.get(); }

        /**
         * @brief Update the active scene
         */
        void Update(float deltaTime);

        /**
         * @brief Render the active scene
         */
        void Render();

        /**
         * @brief Render ImGui for the active scene
         */
        void RenderImGui();

    private:
        Graphics::GraphicsDevice& m_Device;
        std::unordered_map<std::string, SceneFactory> m_SceneFactories;
        std::unique_ptr<IScene> m_ActiveScene;
        std::string m_ActiveSceneName;
    };

} // namespace Yamen::Client
