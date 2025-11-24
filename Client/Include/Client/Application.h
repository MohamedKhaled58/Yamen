#pragma once

#include "Platform/Window.h"
#include "Platform/Layers/Layer.h"
#include "Platform/Events/Event.h"
#include "Platform/Events/InputDispatcher.h"
#include "Graphics/RHI/GraphicsDevice.h"
#include "Graphics/RHI/SwapChain.h"
#include <memory>

namespace Yamen::Client {

    class ImGuiLayer;

    /**
     * @brief Main application class
     * 
     * Manages window, graphics device, layer stack, and main loop.
     */
    class Application {
    public:
        Application();
        ~Application();

        /**
         * @brief Run the application
         */
        void Run();

        /**
         * @brief Get singleton instance
         */
        static Application& Get() { return *s_Instance; }

        /**
         * @brief Get graphics device
         */
        Graphics::GraphicsDevice& GetGraphicsDevice() { return *m_GraphicsDevice; }

        /**
         * @brief Get swap chain
         */
        Graphics::SwapChain& GetSwapChain() { return *m_SwapChain; }

        /**
         * @brief Get window
         */
        Platform::Window& GetWindow() { return *m_Window; }

        /**
         * @brief Get layer stack
         */
        Platform::LayerStack& GetLayerStack() { return *m_LayerStack; }

    private:
        void OnEvent(Platform::Event& event);

        std::unique_ptr<Platform::Window> m_Window;
        std::unique_ptr<Graphics::GraphicsDevice> m_GraphicsDevice;
        std::unique_ptr<Graphics::SwapChain> m_SwapChain;
        std::unique_ptr<Platform::LayerStack> m_LayerStack;
        ImGuiLayer* m_ImGuiLayer = nullptr;
        Platform::EventDispatcher m_EventDispatcher;
        Platform::InputDispatcher m_InputDispatcher;

        static Application* s_Instance;
    };

} // namespace Yamen::Client
