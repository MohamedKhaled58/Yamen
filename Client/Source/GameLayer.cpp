#include "Client/GameLayer.h"
#include "Client/Application.h"
#include "Core/Logging/Logger.h"
#include "Graphics/Texture/TextureLoader.h"
#include "Platform/Input.h"
#include <imgui.h>

namespace Yamen::Client {

    GameLayer::GameLayer()
        : Layer("GameLayer")
    {
    }

    void GameLayer::OnAttach() {
        YAMEN_CLIENT_INFO("GameLayer attached");

        // Create camera
        auto& window = Application::Get().GetWindow();
        m_Camera = std::make_unique<Graphics::Camera2D>(
            static_cast<float>(window.GetWidth()),
            static_cast<float>(window.GetHeight())
        );

        // Create test texture (white square)
        auto& device = Application::Get().GetGraphicsDevice();
        m_TestTexture = Graphics::TextureLoader::CreateSolidColor(device, 64, 64, 255, 255, 255, 255);
    }

    void GameLayer::OnDetach() {
        YAMEN_CLIENT_INFO("GameLayer detached");
    }

    void GameLayer::OnUpdate(float deltaTime) {
        // Camera controls (WASD)
        glm::vec2 movement(0.0f);

        if (Platform::Input::IsKeyPressed(Platform::KeyCode::W)) {
            movement.y += 1.0f;
        }
        if (Platform::Input::IsKeyPressed(Platform::KeyCode::S)) {
            movement.y -= 1.0f;
        }
        if (Platform::Input::IsKeyPressed(Platform::KeyCode::A)) {
            movement.x -= 1.0f;
        }
        if (Platform::Input::IsKeyPressed(Platform::KeyCode::D)) {
            movement.x += 1.0f;
        }

        if (glm::length(movement) > 0.0f) {
            movement = glm::normalize(movement);
            m_Camera->SetPosition(m_Camera->GetPosition() + movement * m_CameraSpeed * deltaTime);
        }

        // Zoom controls (Q/E)
        if (Platform::Input::IsKeyPressed(Platform::KeyCode::Q)) {
            m_Camera->SetZoom(m_Camera->GetZoom() - deltaTime);
        }
        if (Platform::Input::IsKeyPressed(Platform::KeyCode::E)) {
            m_Camera->SetZoom(m_Camera->GetZoom() + deltaTime);
        }
    }

    void GameLayer::OnRender() {
        // TODO: Implement 2D rendering with SpriteBatch
        // For now, just clear screen (done in Application)
    }

    void GameLayer::OnEvent(Platform::Event& event) {
        // Handle events
    }

    void GameLayer::OnImGuiRender() {
        // Debug UI
        ImGui::Begin("Game Debug");

        ImGui::Text("Camera");
        ImGui::Separator();

        glm::vec2 camPos = m_Camera->GetPosition();
        if (ImGui::DragFloat2("Position", &camPos.x, 1.0f)) {
            m_Camera->SetPosition(camPos);
        }

        float zoom = m_Camera->GetZoom();
        if (ImGui::SliderFloat("Zoom", &zoom, 0.1f, 5.0f)) {
            m_Camera->SetZoom(zoom);
        }

        ImGui::SliderFloat("Camera Speed", &m_CameraSpeed, 100.0f, 1000.0f);

        ImGui::Separator();
        ImGui::Text("Controls:");
        ImGui::BulletText("WASD - Move camera");
        ImGui::BulletText("Q/E - Zoom in/out");

        ImGui::End();
    }

} // namespace Yamen::Client
