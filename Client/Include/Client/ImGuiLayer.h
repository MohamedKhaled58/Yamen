#pragma once

#include "Platform/Layers/Layer.h"

namespace Yamen::Client {

    /**
     * @brief ImGui layer for debug UI
     * 
     * Manages ImGui lifecycle, backend initialization, and rendering.
     */
    class ImGuiLayer : public Platform::Layer {
    public:
        ImGuiLayer();
        ~ImGuiLayer() override;

        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate(float deltaTime) override;
        void OnRender() override;
        void OnImGuiRender() override;
        void OnEvent(Platform::Event& event) override;

        /**
         * @brief Begin ImGui frame
         * Called before layer updates
         */
        void Begin();

        /**
         * @brief End ImGui frame and render
         * Called after layer rendering
         */
        void End();

    private:
        bool m_BlockEvents = true;
    };

} // namespace Yamen::Client
