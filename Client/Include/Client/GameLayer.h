#pragma once

#include "Platform/Layers/Layer.h"
#include "Client/SceneManager.h"
#include <memory>

namespace Yamen::Client {

    /**
     * @brief Main game layer that manages scenes
     * 
     * Owns the SceneManager and delegates to the active scene.
     * Simplified from managing multiple scenes directly.
     */
    class GameLayer : public Platform::Layer {
    public:
        GameLayer();

        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate(float deltaTime) override;
        void OnRender() override;
        void OnEvent(Platform::Event& event) override;
        void OnImGuiRender() override;

    private:
        std::unique_ptr<SceneManager> m_SceneManager;
    };

} // namespace Yamen::Client
