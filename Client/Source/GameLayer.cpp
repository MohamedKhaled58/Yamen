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

#if ENABLE_DEMO_SCENE
        // === DEMO SCENE INITIALIZATION ===
        m_DemoScene = std::make_unique<DemoScene>(device);
        if (!m_DemoScene->Initialize()) {
            YAMEN_CLIENT_ERROR("Failed to initialize demo scene");
        }
#endif
    }

    void GameLayer::OnDetach() {
        YAMEN_CLIENT_INFO("GameLayer detached");

#if ENABLE_DEMO_SCENE
        m_DemoScene.reset();
#endif
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

#if ENABLE_DEMO_SCENE
        // === UPDATE DEMO SCENE ===
        m_DemoScene->Update(deltaTime);
#endif
    }

    void GameLayer::OnRender() {
#if ENABLE_DEMO_SCENE
        // === RENDER DEMO SCENE ===
        m_DemoScene->Render();
#else
        // Production rendering code goes here
        // TODO: Implement game rendering
#endif
    }

    void GameLayer::OnEvent(Platform::Event& event) {
        // Handle events
    }

    void GameLayer::OnImGuiRender() {
#if ENABLE_DEMO_SCENE
        // === DEMO SCENE UI ===
         m_DemoScene->RenderImGui(); // Disable internal demo UI to avoid duplication
        
        if (m_DemoScene) {
            auto* camera = m_DemoScene->GetCamera3D();
            if (camera) {
                ImGui::Begin("Scene Controls");
                
                ImGui::Text("Camera 3D");
                ImGui::Separator();
                
                // Position
                glm::vec3 pos = camera->GetPosition();
                if (ImGui::DragFloat3("Position", &pos.x, 0.1f)) {
                    camera->SetPosition(pos);
                }
                
                // Rotation
                glm::vec3 rot = camera->GetRotation();
                // Convert to degrees for display
                glm::vec3 rotDeg = glm::degrees(rot);
                if (ImGui::DragFloat3("Rotation", &rotDeg.x, 1.0f)) {
                    camera->SetRotation(glm::radians(rotDeg));
                }

                // FOV
                float fov = camera->GetFOV();
                if (ImGui::SliderFloat("FOV", &fov, 30.0f, 120.0f)) {
                    camera->SetFOV(fov);
                }
                
                ImGui::Separator();
                
                if (ImGui::Button("Reset Camera")) {
                    camera->SetPosition(glm::vec3(0.0f, 5.0f, -15.0f));
                    camera->SetRotation(glm::vec3(glm::radians(-15.0f), glm::radians(90.0f), 0.0f));
                    camera->SetFOV(60.0f);
                }

                ImGui::End();
            }
        }
#endif
    }

} // namespace Yamen::Client
