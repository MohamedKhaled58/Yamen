#include "Client/Application.h"
#include "Client/GameLayer.h"
#include "Client/ImGuiLayer.h"
#include "Core/Logging/Logger.h"
#include "Graphics/RHI/RenderTarget.h"
#include "Platform/Timer.h"

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

            // Get back buffer
            auto* backBuffer = m_SwapChain->GetBackBuffer();
            if (!backBuffer) {
                continue;
            }

            // ✅ FIX: Set render target BEFORE clearing
            auto* rtv = backBuffer->GetRTV();
            m_GraphicsDevice->GetContext()->OMSetRenderTargets(1, &rtv, nullptr);

            // Clear back buffer
            backBuffer->Clear(0.1f, 0.1f, 0.1f, 1.0f);

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
        // Dispatch to global event dispatcher first
        m_EventDispatcher.Dispatch(event);

        // Propagate to layers (in reverse order for proper event handling)
        m_LayerStack->OnEvent(event);
    }

} // namespace Yamen::Client