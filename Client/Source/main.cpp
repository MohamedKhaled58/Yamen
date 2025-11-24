#include "Core/Logging/Logger.h"
#include "Platform/Window.h"
#include "Platform/Timer.h"
#include "Platform/Events/ApplicationEvents.h"
#include "Platform/Events/InputEvents.h"
#include "Platform/Events/InputDispatcher.h"
#include "Platform/Layers/Layer.h"
#include "Graphics/Graphics.h"

using namespace Yamen;

/**
 * @brief Example game layer
 */
class GameLayer : public Platform::Layer {
public:
    GameLayer() : Layer("GameLayer") {}

    void OnAttach() override {
        YAMEN_CLIENT_INFO("GameLayer attached");
    }

    void OnDetach() override {
        YAMEN_CLIENT_INFO("GameLayer detached");
    }

    void OnUpdate(float deltaTime) override {
        // Game logic update
    }

    void OnRender() override {
        // Game rendering
    }

    void OnEvent(Platform::Event& event) override {
        if (event.GetCategory() == Platform::EventCategory::Keyboard) {
            auto& keyEvent = event.As<Platform::KeyPressedEvent>();
            Platform::KeyCode key = keyEvent.GetKey();
            YAMEN_CLIENT_INFO("Key pressed: {}", static_cast<int>(key));
        }
    }
};

/**
 * @brief Main application class
 */
class Application {
public:
    Application()
        : m_InputDispatcher(m_EventDispatcher)
    {
        // Initialize logger
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
        m_LayerStack->PushLayer(std::make_unique<GameLayer>());

        YAMEN_CLIENT_INFO("Application initialized");
    }

    ~Application() {
        YAMEN_CLIENT_INFO("Application shutting down");
        
        // Shutdown graphics
        m_SwapChain.reset();
        m_GraphicsDevice->Shutdown();
    }

    void Run() {
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
            // Clear back buffer to cornflower blue
            auto* backBuffer = m_SwapChain->GetBackBuffer();
            if (backBuffer) {
                backBuffer->Clear(0.39f, 0.58f, 0.93f, 1.0f); // Cornflower blue
            }

            // Render layers
            m_LayerStack->OnRender();

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

private:
    void OnEvent(Platform::Event& event) {
        // Dispatch to global event dispatcher first
        m_EventDispatcher.Dispatch(event);

        // Propagate to layers (in reverse order for proper event handling)
        m_LayerStack->OnEvent(event);
    }

    std::unique_ptr<Platform::Window> m_Window;
    std::unique_ptr<Platform::LayerStack> m_LayerStack;
    std::unique_ptr<Graphics::GraphicsDevice> m_GraphicsDevice;
    std::unique_ptr<Graphics::SwapChain> m_SwapChain;
    Platform::EventDispatcher m_EventDispatcher;
    Platform::InputDispatcher m_InputDispatcher;
};

/**
 * @brief Entry point
 */
int main(int argc, char** argv) {
    try {
        Application app;
        app.Run();
        return 0;
    }
    catch (const std::exception& e) {
        YAMEN_CLIENT_CRITICAL("Fatal error: {}", e.what());
        return -1;
    }
}