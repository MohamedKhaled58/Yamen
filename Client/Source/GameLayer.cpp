#include "Client/GameLayer.h"
#include "Client/Application.h"
#include "Platform/Input.h"
#include <Core/Logging/Logger.h>
#include <imgui.h>


// Include scene headers for registration
#include "Client/DemoScene.h"
#include "Client/ECSScene.h"
#include "Client/LightingDemoScene.h"
#include "Client/MultiCameraScene.h"
#include "Client/PhysicsPlaygroundScene.h"
#include "Client/Scenes/C3AnimationDemoScene.h"


namespace Yamen::Client {

GameLayer::GameLayer() : Layer("GameLayer") {}

void GameLayer::OnAttach() {
  YAMEN_CLIENT_INFO("GameLayer attached");

  auto &device = Application::Get().GetGraphicsDevice();

  // Initialize Scene Manager
  m_SceneManager = std::make_unique<SceneManager>(device);

  // Register Scenes
  m_SceneManager->RegisterScene("ECS Scene",
                                [&device]() -> std::unique_ptr<IScene> {
                                  return std::make_unique<ECSScene>(device);
                                });

  m_SceneManager->RegisterScene(
      "Physics Playground", [&device]() -> std::unique_ptr<IScene> {
        return std::make_unique<PhysicsPlaygroundScene>(device);
      });

  m_SceneManager->RegisterScene(
      "Lighting Demo", [&device]() -> std::unique_ptr<IScene> {
        return std::make_unique<LightingDemoScene>(device);
      });

  m_SceneManager->RegisterScene(
      "Multi-Camera Demo", [&device]() -> std::unique_ptr<IScene> {
        return std::make_unique<MultiCameraScene>(device);
      });

  m_SceneManager->RegisterScene("Legacy Demo",
                                [&device]() -> std::unique_ptr<IScene> {
                                  return std::make_unique<DemoScene>(device);
                                });

  m_SceneManager->RegisterScene(
      "C3 Animation Demo", [&device]() -> std::unique_ptr<IScene> {
        return std::make_unique<C3AnimationDemoScene>(device);
      });

  // Load default scene
  if (!m_SceneManager->LoadScene("ECS Scene")) {
    YAMEN_CLIENT_ERROR("Failed to load default scene");
  }
}

void GameLayer::OnDetach() {
  YAMEN_CLIENT_INFO("GameLayer detached");
  m_SceneManager.reset();
}

void GameLayer::OnUpdate(float deltaTime) {
  if (m_SceneManager) {
    m_SceneManager->Update(deltaTime);
  }
}

void GameLayer::OnRender() {
  if (m_SceneManager) {
    m_SceneManager->Render();
  }
}

void GameLayer::OnEvent(Platform::Event &event) {
  // Pass events to scene manager if needed
}

void GameLayer::OnImGuiRender() {
  if (m_SceneManager) {
    m_SceneManager->RenderImGui();

    // Scene Selection UI - with flags to keep it on top and prevent focus
    // issues
    ImGui::Begin("Scene Manager", nullptr,
                 ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_AlwaysAutoResize);

    const char *scenes[] = {"ECS Scene",     "Physics Playground",
                            "Lighting Demo", "Multi-Camera Demo",
                            "Legacy Demo",   "C3 Animation Demo"};
    static int currentScene = 0;

    if (ImGui::Combo("Active Scene", &currentScene, scenes,
                     IM_ARRAYSIZE(scenes))) {
      m_SceneManager->LoadScene(scenes[currentScene]);
    }

    if (m_SceneManager->GetActiveScene()) {
      ImGui::Text("Current: %s", m_SceneManager->GetActiveScene()->GetName());
    }

    ImGui::End();
  }
}

} // namespace Yamen::Client
