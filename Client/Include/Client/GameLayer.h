#pragma once

#include "Platform/Layers/Layer.h"
#include "Graphics/Texture/Texture2D.h"
#include "Graphics/Renderer/Camera2D.h"
#include <memory>

namespace Yamen::Client {

    /**
     * @brief Main game layer
     * 
     * Handles game logic, rendering, and input.
     */
    class GameLayer : public Platform::Layer {
    public:
        GameLayer();
        ~GameLayer() override = default;

        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate(float deltaTime) override;
        void OnRender() override;
        void OnEvent(Platform::Event& event) override;
        void OnImGuiRender() override;

    private:
        std::unique_ptr<Graphics::Camera2D> m_Camera;
        std::unique_ptr<Graphics::Texture2D> m_TestTexture;
        float m_CameraSpeed = 500.0f;
    };

} // namespace Yamen::Client
