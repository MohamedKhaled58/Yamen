#include "Client/ImGuiLayer.h"
#include "Client/Application.h"
#include "Core/Logging/Logger.h"

#include <imgui.h>
#include <./backends/imgui_impl_win32.h>
#include <./backends/imgui_impl_dx11.h>

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Yamen::Client {

    ImGuiLayer::ImGuiLayer()
        : Layer("ImGuiLayer")
    {
    }

    ImGuiLayer::~ImGuiLayer() {
    }

    void ImGuiLayer::OnAttach() {
        YAMEN_CLIENT_INFO("ImGuiLayer attached");

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        // Setup Platform/Renderer backends
        auto& app = Application::Get();
        auto& window = app.GetWindow();
        auto& device = app.GetGraphicsDevice();

        ImGui_ImplWin32_Init(window.GetNativeWindow());
        ImGui_ImplDX11_Init(device.GetDevice(), device.GetContext());

        YAMEN_CLIENT_INFO("ImGui initialized (Win32 + DX11 backends)");
    }

    void ImGuiLayer::OnDetach() {
        YAMEN_CLIENT_INFO("ImGuiLayer detached");

        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::OnUpdate(float deltaTime) {
        // ImGui update is handled in Begin()
    }

    void ImGuiLayer::OnRender() {
        // ImGui rendering is handled in End()
    }

    void ImGuiLayer::OnImGuiRender() {
        // Demo window for testing
        static bool showDemo = true;
        if (showDemo) {
            ImGui::ShowDemoWindow(&showDemo);
        }
    }

    void ImGuiLayer::OnEvent(Platform::Event& event) {
        if (m_BlockEvents) {
            ImGuiIO& io = ImGui::GetIO();
            // Block events if ImGui wants to capture them
            if (io.WantCaptureMouse) {
                // event.Handled = true; // TODO: Add handled flag to Event
            }
            if (io.WantCaptureKeyboard) {
                // event.Handled = true;
            }
        }
    }

    void ImGuiLayer::Begin() {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiLayer::End() {
        ImGuiIO& io = ImGui::GetIO();
        auto& app = Application::Get();
        io.DisplaySize = ImVec2(
            static_cast<float>(app.GetWindow().GetWidth()),
            static_cast<float>(app.GetWindow().GetHeight())
        );

        // Rendering
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

} // namespace Yamen::Client
