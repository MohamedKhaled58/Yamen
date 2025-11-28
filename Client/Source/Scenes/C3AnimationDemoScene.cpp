#include "Client/Scenes/C3AnimationDemoScene.h"
#include "Core/Logging/Logger.h"
#include "Platform/Input.h"
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

namespace Yamen {

C3AnimationDemoScene::C3AnimationDemoScene(Graphics::GraphicsDevice &device)
    : m_Device(device), m_CurrentModelIndex(0), m_CameraDistance(200.0f),
      m_CameraAngle(0.0f), m_CameraHeight(0.0f), m_AnimationPaused(false),
      m_AnimationSpeed(30.0f), m_ModelScale(1.0f) {}

C3AnimationDemoScene::~C3AnimationDemoScene() {}

bool C3AnimationDemoScene::Initialize() {
  YAMEN_CORE_INFO("C3AnimationDemoScene: Initializing...");

  // Create camera
  m_Camera = std::make_unique<Graphics::Camera3D>(45.0f, 1920.0f / 1080.0f,
                                                  0.1f, 1000.0f);

  // Create skeletal renderer
  m_SkeletalRenderer = std::make_unique<Graphics::C3SkeletalRenderer>(m_Device);
  if (!m_SkeletalRenderer->Initialize()) {
    YAMEN_CORE_ERROR(
        "C3AnimationDemoScene: Failed to initialize C3SkeletalRenderer!");
    return false;
  }

  // Define ghost models (different animation states)
  m_GhostModels = {
      {entt::null, "Base Model",
       "C:/dev/C3Renderer/Yamen/Assets/C3/ghost/140/1.c3", false, nullptr},
      {entt::null, "Standby",
       "C:/dev/C3Renderer/Yamen/Assets/C3/ghost/140/100.c3", false, nullptr},
      {entt::null, "Rest", "C:/dev/C3Renderer/Yamen/Assets/C3/ghost/140/101.c3",
       false, nullptr},
      {entt::null, "Walk Left",
       "C:/dev/C3Renderer/Yamen/Assets/C3/ghost/140/110.c3", false, nullptr},
      {entt::null, "Walk Right",
       "C:/dev/C3Renderer/Yamen/Assets/C3/ghost/140/111.c3", false, nullptr},
      {entt::null, "Run Left",
       "C:/dev/C3Renderer/Yamen/Assets/C3/ghost/140/120.c3", false, nullptr},
      {entt::null, "Run Right",
       "C:/dev/C3Renderer/Yamen/Assets/C3/ghost/140/121.c3", false, nullptr},
      {entt::null, "Attack",
       "C:/dev/C3Renderer/Yamen/Assets/C3/ghost/140/321.c3", false, nullptr},
      {entt::null, "Unknown",
       "C:/dev/C3Renderer/Yamen/Assets/C3/ghost/140/331.c3", false, nullptr}};

  // Load all models
  LoadAllModels();

  // Start with base model (visible geometry)
  SwitchToModel(0);

  YAMEN_CORE_INFO("C3AnimationDemoScene: Ready! Loaded {} ghost animations",
                  m_GhostModels.size());

  return true;
}

void C3AnimationDemoScene::LoadAllModels() {
  // 1. Load the Base Model (Geometry)
  // This is the only entity we will render.
  m_BaseEntity =
      Client::C3ModelLoader::LoadModel(m_Registry, m_GhostModels[0].filepath);
  if (m_BaseEntity != entt::null) {
    m_GhostModels[0].isLoaded = true;
    m_GhostModels[0].entity = m_BaseEntity;

    // Store its motion as well
    auto *animComp =
        m_Registry.try_get<ECS::SkeletalAnimationComponent>(m_BaseEntity);
    if (animComp) {
      m_GhostModels[0].motion = animComp->motion;
    }
  } else {
    YAMEN_CORE_ERROR("Failed to load Base Model: {}",
                     m_GhostModels[0].filepath);
  }

  // 2. Load other files as "Animation Sources"
  // We load them temporarily to extract the C3Motion data
  for (size_t i = 1; i < m_GhostModels.size(); ++i) {
    auto &model = m_GhostModels[i];

    // Load as a temporary entity
    entt::entity tempEntity =
        Client::C3ModelLoader::LoadModel(m_Registry, model.filepath);
    if (tempEntity != entt::null) {
      model.isLoaded = true;
      model.entity = entt::null; // Not a renderable entity

      // Extract motion
      auto *animComp =
          m_Registry.try_get<ECS::SkeletalAnimationComponent>(tempEntity);
      if (animComp && animComp->motion) {
        model.motion = animComp->motion;
        YAMEN_CORE_INFO("Loaded animation '{}' from {}", model.name,
                        model.filepath);
      } else {
        YAMEN_CORE_WARN("Model '{}' has no animation data", model.name);
      }

      // Hide the temporary entity so it doesn't render (it has no vertices
      // anyway)
      auto *meshComp = m_Registry.try_get<ECS::C3MeshComponent>(tempEntity);
      if (meshComp) {
        meshComp->visible = false;
      }

      // We keep the entity alive because it owns the C3Phy which owns C3Motion
      model.entity = tempEntity;
    }
  }
}

void C3AnimationDemoScene::Update(float deltaTime) {
  // Camera controls
  if (Platform::Input::IsKeyPressed(Platform::KeyCode::Left)) {
    m_CameraAngle -= 90.0f * deltaTime;
  }
  if (Platform::Input::IsKeyPressed(Platform::KeyCode::Right)) {
    m_CameraAngle += 90.0f * deltaTime;
  }
  if (Platform::Input::IsKeyPressed(Platform::KeyCode::Up)) {
    m_CameraDistance = std::max(10.0f, m_CameraDistance - 50.0f * deltaTime);
  }
  if (Platform::Input::IsKeyPressed(Platform::KeyCode::Down)) {
    m_CameraDistance = std::min(500.0f, m_CameraDistance + 50.0f * deltaTime);
  }

  // Animation controls
  if (Platform::Input::IsKeyPressed(Platform::KeyCode::Space)) {
    m_AnimationPaused = !m_AnimationPaused;
  }

  // Model switching (1-8 keys)
  if (Platform::Input::IsKeyPressed(Platform::KeyCode::Num1))
    SwitchToModel(0);
  if (Platform::Input::IsKeyPressed(Platform::KeyCode::Num2))
    SwitchToModel(1);
  if (Platform::Input::IsKeyPressed(Platform::KeyCode::Num3))
    SwitchToModel(2);
  if (Platform::Input::IsKeyPressed(Platform::KeyCode::Num4))
    SwitchToModel(3);
  if (Platform::Input::IsKeyPressed(Platform::KeyCode::Num5))
    SwitchToModel(4);
  if (Platform::Input::IsKeyPressed(Platform::KeyCode::Num6))
    SwitchToModel(5);
  if (Platform::Input::IsKeyPressed(Platform::KeyCode::Num7))
    SwitchToModel(6);
  if (Platform::Input::IsKeyPressed(Platform::KeyCode::Num8))
    SwitchToModel(7);

  // Update camera
  UpdateCamera();

  // Update animations (if not paused)
  if (!m_AnimationPaused) {
    ECS::SkeletalAnimationSystem::Update(m_Registry, deltaTime);
  }
}

void C3AnimationDemoScene::Render() {
  if (!m_SkeletalRenderer || m_CurrentModelIndex < 0 ||
      m_CurrentModelIndex >= m_GhostModels.size()) {
    return;
  }

  auto &currentModel = m_GhostModels[m_CurrentModelIndex];
  if (!currentModel.isLoaded || currentModel.entity == entt::null) {
    return;
  }

  // Calculate MVP matrix
  glm::mat4 model = glm::mat4(1.0f);
  glm::mat4 view = m_Camera->GetViewMatrix();
  glm::mat4 projection = m_Camera->GetProjectionMatrix();
  glm::mat4 mvp = projection * view * model;

  // Render the current ghost model
  Client::C3ModelLoader::RenderModel(currentModel.entity, m_Registry,
                                     *m_SkeletalRenderer, mvp);
}

void C3AnimationDemoScene::RenderImGui() {
  ImGui::Begin("C3 Animation Demo");

  ImGui::Text("Ghost Character Animations");
  ImGui::Separator();

  // Model selection
  ImGui::Text("Select Animation:");
  for (int i = 0; i < m_GhostModels.size(); ++i) {
    auto &model = m_GhostModels[i];

    bool isSelected = (i == m_CurrentModelIndex);
    if (ImGui::Selectable(model.name.c_str(), isSelected)) {
      SwitchToModel(i);
    }

    // Show load status
    ImGui::SameLine();
    if (model.isLoaded) {
      ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[Loaded]");
    } else {
      ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[Failed]");
    }
  }

  ImGui::Separator();

  // Animation controls
  ImGui::Text("Animation Controls:");
  if (ImGui::Button(m_AnimationPaused ? "Play (Space)" : "Pause (Space)")) {
    m_AnimationPaused = !m_AnimationPaused;
  }

  ImGui::SliderFloat("Speed (FPS)", &m_AnimationSpeed, 1.0f, 120.0f);

  // Apply speed to current animation
  if (m_CurrentModelIndex >= 0 && m_CurrentModelIndex < m_GhostModels.size()) {
    auto &model = m_GhostModels[m_CurrentModelIndex];
    if (model.isLoaded && model.entity != entt::null) {
      auto *anim =
          m_Registry.try_get<ECS::SkeletalAnimationComponent>(model.entity);
      if (anim) {
        ECS::SkeletalAnimationSystem::SetSpeed(*anim, m_AnimationSpeed);

        // Show animation info
        ImGui::Separator();
        ImGui::Text("Current Animation:");
        ImGui::Text("  Frame: %.1f / %d", anim->currentFrame,
                    anim->motion ? anim->motion->frameCount : 0);
        ImGui::Text("  Bones: %d", anim->motion ? anim->motion->boneCount : 0);
        ImGui::Text("  Keyframes: %d",
                    anim->motion ? anim->motion->keyframeCount : 0);

        if (anim->motion) {
          const char *formatNames[] = {"Legacy", "KKEY", "XKEY", "ZKEY"};
          int formatIndex = static_cast<int>(anim->motion->format);
          ImGui::Text("  Format: %s", formatNames[formatIndex]);
        }
      }
    }
  }

  ImGui::Separator();

  // Camera controls
  ImGui::Text("Camera Controls:");
  ImGui::Text("  Arrow Keys: Rotate/Zoom");
  ImGui::SliderFloat("Distance", &m_CameraDistance, 10.0f, 500.0f);
  ImGui::SliderFloat("Angle", &m_CameraAngle, -180.0f, 180.0f);
  ImGui::SliderFloat("Height", &m_CameraHeight, -100.0f, 100.0f);
  ImGui::SliderFloat("Scale", &m_ModelScale, 0.01f, 5.0f);

  ImGui::Separator();
  ImGui::Text("Hotkeys:");
  ImGui::BulletText("1-8: Switch animations");
  ImGui::BulletText("Space: Pause/Play");
  ImGui::BulletText("Arrows: Camera control");

  ImGui::End();
}

void C3AnimationDemoScene::LoadAllModels() {
  YAMEN_CORE_INFO("Loading {} ghost models...", m_GhostModels.size());

  for (auto &model : m_GhostModels) {
    model.entity = Client::C3ModelLoader::LoadModel(m_Registry, model.filepath);
    model.isLoaded = (model.entity != entt::null);

    if (model.isLoaded) {
      YAMEN_CORE_INFO("  ✓ Loaded: {}", model.name);

      // Set animation to loop and pause initially
      auto *anim =
          m_Registry.try_get<ECS::SkeletalAnimationComponent>(model.entity);
      if (anim) {
        anim->loop = true;
        anim->isPlaying = false;
        anim->playbackSpeed = m_AnimationSpeed;
      }
    } else {
      YAMEN_CORE_ERROR("  ✗ Failed: {}", model.name);
    }
  }
}

void C3AnimationDemoScene::UnloadAllModels() {
  for (auto &model : m_GhostModels) {
    if (model.isLoaded && model.entity != entt::null) {
      Client::C3ModelLoader::UnloadModel(model.entity, m_Registry);
      model.entity = entt::null;
      model.isLoaded = false;
    }
  }
}

void C3AnimationDemoScene::SwitchToModel(int index) {
  if (index < 0 || index >= m_GhostModels.size()) {
    return;
  }

  // Pause all animations
  for (auto &model : m_GhostModels) {
    if (model.isLoaded && model.entity != entt::null) {
      auto *anim =
          m_Registry.try_get<ECS::SkeletalAnimationComponent>(model.entity);
      if (anim) {
        ECS::SkeletalAnimationSystem::Stop(*anim);
      }
    }
  }

  // Switch to new model
  m_CurrentModelIndex = index;
  auto &newModel = m_GhostModels[index];

  if (newModel.isLoaded && newModel.entity != entt::null) {
    auto *anim =
        m_Registry.try_get<ECS::SkeletalAnimationComponent>(newModel.entity);
    if (anim) {
      ECS::SkeletalAnimationSystem::Play(*anim, true);
      ECS::SkeletalAnimationSystem::SetSpeed(*anim, m_AnimationSpeed);
    }

    YAMEN_CORE_INFO("Switched to: {}", newModel.name);
  }
}

void C3AnimationDemoScene::UpdateCamera() {
  // Orbit camera around the model
  float angleRad = glm::radians(m_CameraAngle);

  glm::vec3 position;
  position.x = m_CameraDistance * sin(angleRad);
  position.y = m_CameraHeight;
  position.z = m_CameraDistance * cos(angleRad);

  glm::vec3 target(0.0f, 1.0f, 0.0f); // Look at model center

  m_Camera->SetPosition(position);
  // Calculate rotation to look at target
  glm::vec3 direction = glm::normalize(target - position);
  float pitch = asin(-direction.y);
  float yaw = atan2(direction.x, direction.z);
  m_Camera->SetRotation(pitch, yaw, 0.0f);
}

} // namespace Yamen
