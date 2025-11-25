#include "Client/Application.h"
#include "Client/GameLayer.h"
#include "Client/ImGuiLayer.h"
#include "Core/Logging/Logger.h"
#include "Graphics/RHI/RenderTarget.h"
#include "Graphics/RHI/DepthStencilBuffer.h"
#include "Platform/Timer.h"
#include "Platform/Events/ApplicationEvents.h"

namespace Yamen::Client {

    Application* Application::s_Instance = nullptr;

    Application::Application()
        : m_InputDispatcher(m_EventDispatcher)
    {
        // Set singleton
        s_Instance = this;
        YAMEN_CLIENT_INFO("=== Yamen Engine Starting ===");

        // Create window
        Platform::WindowProps props;
        props.title = "Yamen Engine - Conquer Online 2.0 Client";
        props.width = 1280;
        props.height = 720;
        props.vsync = true;

        m_Window = std::make_unique<Platform::Window>(props);
        m_Window->SetEventCallback([this](Platform::Event& e) { OnEvent(e); });

        // Initialize graphics
        m_GraphicsDevice = std::make_unique<Graphics::GraphicsDevice>();
        if (!m_GraphicsDevice->Initialize(true)) {
            YAMEN_CLIENT_CRITICAL("Failed to initialize graphics device");
            return;
        }

        // Create swap chain
        m_SwapChain = std::make_unique<Graphics::SwapChain>(*m_GraphicsDevice);
        if (!m_SwapChain->Create(m_Window->GetNativeHandle(), props.width, props.height, props.vsync)) {
            YAMEN_CLIENT_CRITICAL("Failed to create swap chain");
            return;
        }

        // Create layer stack
        m_LayerStack = std::make_unique<Platform::LayerStack>();

        // Add game layer
        m_LayerStack->PushLayer(std::make_unique<GameLayer>());

        // Add ImGui layer as overlay (renders on top)
        m_ImGuiLayer = new ImGuiLayer();
        m_LayerStack->PushOverlay(std::unique_ptr<Platform::Layer>(m_ImGuiLayer));

        YAMEN_CLIENT_INFO("Application initialized");
    }

    Application::~Application() {
        YAMEN_CLIENT_INFO("Application shutting down");

        // Shutdown graphics
        m_SwapChain.reset();
        m_GraphicsDevice->Shutdown();

        s_Instance = nullptr;
    }

    void Application::Run() {
        YAMEN_CLIENT_INFO("Entering main loop");

        Platform::FrameTimer frameTimer;

        while (!m_Window->ShouldClose()) {
            // Update timer
            float deltaTime = frameTimer.Update();

            // Poll input and dispatch events
            m_InputDispatcher.Update();

            // Update window (process messages)
            m_Window->OnUpdate();

            // Update layers
            m_LayerStack->OnUpdate(deltaTime);

            // === RENDERING ===

            // Get back buffer and depth buffer
            auto* backBuffer = m_SwapChain->GetBackBuffer();
            auto* depthBuffer = m_SwapChain->GetDepthBuffer();
            if (!backBuffer) {
                continue;
            }

            // Set render target and depth buffer
            auto* rtv = backBuffer->GetRTV();
            auto* dsv = depthBuffer ? depthBuffer->GetDSV() : nullptr;
            m_GraphicsDevice->GetContext()->OMSetRenderTargets(1, &rtv, dsv);

            // Set viewport
            D3D11_VIEWPORT viewport = {};
            viewport.TopLeftX = 0.0f;
            viewport.TopLeftY = 0.0f;
            viewport.Width = static_cast<float>(m_Window->GetWidth());
            viewport.Height = static_cast<float>(m_Window->GetHeight());
            viewport.MinDepth = 0.0f;
            viewport.MaxDepth = 1.0f;
            m_GraphicsDevice->GetContext()->RSSetViewports(1, &viewport);

            // Clear back buffer and depth buffer
            backBuffer->Clear(0.1f, 0.1f, 0.1f, 1.0f);
            if (depthBuffer) {
                depthBuffer->Clear();
            }

            // Render layers
            m_LayerStack->OnRender();

            // ImGui rendering
            m_ImGuiLayer->Begin();
            m_LayerStack->OnImGuiRender();
            m_ImGuiLayer->End();

            // Present
            m_SwapChain->Present();

            // Log FPS every second
            static float fpsTimer = 0.0f;
            fpsTimer += deltaTime;
            if (fpsTimer >= 1.0f) {
                YAMEN_CLIENT_INFO("FPS: {:.1f}, Frame Time: {:.2f}ms",
                    frameTimer.GetFPS(), deltaTime * 1000.0f);
                fpsTimer = 0.0f;
            }
        }

        YAMEN_CLIENT_INFO("Exiting main loop");
    }

    void Application::OnEvent(Platform::Event& event) {
        // Handle window resize using dynamic_cast
        if (auto* resizeEvent = dynamic_cast<Platform::WindowResizeEvent*>(&event)) {
            uint32_t width = resizeEvent->GetWidth();
            uint32_t height = resizeEvent->GetHeight();
            
            if (width > 0 && height > 0) {
                // Recreate swap chain with new dimensions
                m_SwapChain->Resize(width, height);
            }
        }

        // Dispatch to global event dispatcher first
        m_EventDispatcher.Dispatch(event);

        // Propagate to layers (in reverse order for proper event handling)
        m_LayerStack->OnEvent(event);
    }

} // namespace Yamen::Client