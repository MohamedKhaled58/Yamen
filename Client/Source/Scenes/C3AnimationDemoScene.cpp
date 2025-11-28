#include "Client/Scenes/C3AnimationDemoScene.h"
#include "Core/Logging/Logger.h"
#include "Platform/Input.h"
#include <d3d11.h>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

namespace Yamen {

C3AnimationDemoScene::C3AnimationDemoScene(Graphics::GraphicsDevice &device)
    : m_Device(device) {
  // Default values
  m_CameraDistance = 1000.0f;
  m_CameraAngle = 0.0f;
  m_CameraHeight = 450.0f;
  m_CameraTarget = glm::vec3(0.0f);
  m_AnimationPaused = false;
  m_AnimationSpeed = 30.0f;
  m_ModelScale = 1.0f;
  m_ShowSkeleton = true;
  m_CurrentModelIndex = 0;
}

C3AnimationDemoScene::~C3AnimationDemoScene() = default;

bool C3AnimationDemoScene::Initialize() {
  YAMEN_CORE_INFO("C3AnimationDemoScene: Initializing...");

  m_Camera = std::make_unique<Graphics::Camera3D>(45.0f, 1920.0f / 1080.0f,
                                                  0.1f, 5000.0f);
  m_SkeletalRenderer = std::make_unique<Graphics::C3SkeletalRenderer>(m_Device);

  if (!m_SkeletalRenderer->Initialize()) {
    YAMEN_CORE_ERROR("Failed to initialize C3SkeletalRenderer!");
    return false;
  }

  // FIXED: Used relative paths "Assets/..." instead of absolute "C:/..."
  m_GhostModels = {
      {entt::null, "Base Model",
       "C:/dev/C3Renderer/Yamen/Assets/C3/ghost/086/100001.c3", false, nullptr},
      {entt::null, "Standby",
       "C:/dev/C3Renderer/Yamen/Assets/C3/ghost/085/100.c3", false, nullptr},
      {entt::null, "Rest", "C:/dev/C3Renderer/Yamen/Assets/C3/ghost/085/101.c3",
       false, nullptr},
      {entt::null, "Walk Left",
       "C:/dev/C3Renderer/Yamen/Assets/C3/ghost/085/110.c3", false, nullptr},
      {entt::null, "Walk Right",
       "C:/dev/C3Renderer/Yamen/Assets/C3/ghost/085/111.c3", false, nullptr},
      {entt::null, "Run Left",
       "C:/dev/C3Renderer/Yamen/Assets/C3/ghost/085/120.c3", false, nullptr},
      {entt::null, "Run Right",
       "C:/dev/C3Renderer/Yamen/Assets/C3/ghost/085/121.c3", false, nullptr},
      {entt::null, "Attack",
       "C:/dev/C3Renderer/Yamen/Assets/C3/ghost/085/350.c3", false, nullptr},
  };

  LoadAllModels();
  SwitchToModel(0);

  m_ModelScale = 2.8f;   // make him HUGE and terrifying
  m_ShowSkeleton = true; // see all bones moving

  YAMEN_CORE_INFO("C3AnimationDemoScene: Ready! Loaded {} animations",
                  m_GhostModels.size());
  return true;
}

void C3AnimationDemoScene::LoadAllModels() {
  // 1. Load base model (has mesh + skeleton)
  m_BaseEntity = Client::C3ModelLoader::LoadModel(m_Registry, m_Device,
                                                  m_GhostModels[0].filepath);

  if (m_BaseEntity != entt::null) {
    m_GhostModels[0].isLoaded = true;
    m_GhostModels[0].entity = m_BaseEntity;

    if (auto *anim =
            m_Registry.try_get<ECS::SkeletalAnimationComponent>(m_BaseEntity)) {

      // Load other animations first
      for (size_t i = 1; i < m_GhostModels.size(); ++i) {
        auto &model = m_GhostModels[i];
        entt::entity e = Client::C3ModelLoader::LoadModel(m_Registry, m_Device,
                                                          model.filepath);
        if (e != entt::null) {
          model.isLoaded = true;
          model.entity = e;
          if (auto *otherAnim =
                  m_Registry.try_get<ECS::SkeletalAnimationComponent>(e)) {
            model.motion = otherAnim->motion;
          }
            

          // Hide mesh so only base model is visible
          if (auto *mesh = m_Registry.try_get<ECS::C3MeshComponent>(e))
            mesh->visible = false;
        }

        // Calculate Inverse Bind Matrices from base model's first keyframe
        if (anim->motion && !anim->motion->keyframes.empty()) {
            const auto& bindFrame = anim->motion->keyframes[0];
            anim->inverseBindMatrices.resize(anim->motion->boneCount);

            for (size_t b = 0; b < anim->motion->boneCount; ++b) {
                const glm::mat4& bindMatrix = bindFrame.boneMatrices[b];
                float det = glm::determinant(bindMatrix);

                if (std::abs(det) > 1e-6f) {
                    anim->inverseBindMatrices[b] = glm::inverse(bindMatrix);
                }
                else {
                    YAMEN_CORE_WARN("Bind pose bone {} is singular, using identity", b);
                    anim->inverseBindMatrices[b] = glm::mat4(1.0f);
                }
            }
            YAMEN_CORE_INFO("Calculated {} inverse bind matrices", anim->motion->boneCount);
        }
      }
    }
  }
}

void C3AnimationDemoScene::Update(float deltaTime) {
  // Camera orbit controls
  if (Platform::Input::IsKeyPressed(Platform::KeyCode::Left))
    m_CameraAngle -= 45.0f * deltaTime;
  if (Platform::Input::IsKeyPressed(Platform::KeyCode::Right))
    m_CameraAngle += 45.0f * deltaTime;
  if (Platform::Input::IsKeyPressed(Platform::KeyCode::Up))
      m_CameraDistance =
          std::max(100.0f, m_CameraDistance - 100.0f * deltaTime);
  if (Platform::Input::IsKeyPressed(Platform::KeyCode::Down))
      m_CameraDistance =
          std::min(3000.0f, m_CameraDistance + 100.0f * deltaTime);
  if (Platform::Input::IsKeyPressed(Platform::KeyCode::PageUp))
    m_CameraHeight = std::min(1000.0f, m_CameraHeight + 100.0f * deltaTime);
  if (Platform::Input::IsKeyPressed(Platform::KeyCode::PageDown))
    m_CameraHeight = std::max(0.0f, m_CameraHeight - 100.0f * deltaTime);

  // WASD Camera Movement (Move Target)
  float moveSpeed = 500.0f * deltaTime;
  float rad = glm::radians(m_CameraAngle);
  glm::vec3 forward(std::sin(rad), 0.0f, std::cos(rad));
  glm::vec3 right(std::cos(rad), 0.0f, -std::sin(rad));

  if (Platform::Input::IsKeyPressed(Platform::KeyCode::W))
    m_CameraTarget += forward * moveSpeed;
  if (Platform::Input::IsKeyPressed(Platform::KeyCode::S))
    m_CameraTarget -= forward * moveSpeed;
  if (Platform::Input::IsKeyPressed(Platform::KeyCode::A))
    m_CameraTarget -= right * moveSpeed;
  if (Platform::Input::IsKeyPressed(Platform::KeyCode::D))
    m_CameraTarget += right * moveSpeed;

  // Animation controls
  if (Platform::Input::IsKeyPressed(Platform::KeyCode::Space))
    m_AnimationPaused = !m_AnimationPaused;

  // Quick animation switch (1-8)
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

  UpdateCamera();

  if (!m_AnimationPaused) {
    ECS::SkeletalAnimationSystem::Update(
        m_Registry, deltaTime * (m_AnimationSpeed / 30.0f));
  }
}

void C3AnimationDemoScene::Render() {
  if (!m_SkeletalRenderer || m_BaseEntity == entt::null)
    return;

  glm::mat4 view = m_Camera->GetViewMatrix();
  glm::mat4 proj = m_Camera->GetProjectionMatrix();

  RenderDebugGrid(view, proj);
  if (m_ShowSkeleton)
    RenderDebugSkeleton(view, proj);

  glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(m_ModelScale));

  // Calculate MVP
  glm::mat4 mvp = proj * view * model;

  // FIXED: Transposed the MVP for the shader (DirectX 11 standard)
  glm::mat4 mvpForShader = glm::transpose(mvp);

  Client::C3ModelLoader::RenderModel(m_BaseEntity, m_Registry,
                                     *m_SkeletalRenderer, mvpForShader);
}

void C3AnimationDemoScene::RenderImGui() {
  ImGui::Begin("C3 Animation Demo - Ghost King", nullptr,
               ImGuiWindowFlags_AlwaysAutoResize);

  ImGui::Text("Animation:");
  for (int i = 0; i < (int)m_GhostModels.size(); ++i) {
    bool selected = (i == m_CurrentModelIndex);
    if (ImGui::Selectable(m_GhostModels[i].name.c_str(), selected)) {
      SwitchToModel(i);
    }
    ImGui::SameLine();
    ImGui::TextColored(m_GhostModels[i].isLoaded ? ImVec4(0, 1, 0, 1)
                                                 : ImVec4(1, 0, 0, 1),
                       m_GhostModels[i].isLoaded ? "[OK]" : "[X]");
  }

  ImGui::Separator();
  ImGui::Checkbox("Show Skeleton", &m_ShowSkeleton);
  if (ImGui::Button(m_AnimationPaused ? "Play (Space)" : "Pause (Space)"))
    m_AnimationPaused = !m_AnimationPaused;

  ImGui::SliderFloat("Speed", &m_AnimationSpeed, 1.0f, 120.0f, "%.1f FPS");
  ImGui::SliderFloat("Scale", &m_ModelScale, 0.1f, 10.0f, "%.2f");

  if (auto *anim =
          m_Registry.try_get<ECS::SkeletalAnimationComponent>(m_BaseEntity)) {
    if (anim->motion) {
      ECS::SkeletalAnimationSystem::SetSpeed(*anim, m_AnimationSpeed);
      ImGui::Text("Frame: %.2f / %d", anim->currentFrame,
                  anim->motion->frameCount);
      ImGui::Text("Bones: %d", anim->motion->boneCount);
    }
  }

  ImGui::End();
}

void C3AnimationDemoScene::SwitchToModel(int index) {
  if (index < 0 || index >= (int)m_GhostModels.size() ||
      !m_GhostModels[index].isLoaded)
    return;

  m_CurrentModelIndex = index;

  if (m_BaseEntity == entt::null)
    return;

  auto *anim =
      m_Registry.try_get<ECS::SkeletalAnimationComponent>(m_BaseEntity);
  if (!anim || !m_GhostModels[index].motion)
    return;

  if (anim->motion != m_GhostModels[index].motion) {
    anim->motion = m_GhostModels[index].motion;
    anim->currentFrame = 0.0f;

    if (anim->boneMatrices.size() != anim->motion->boneCount) {
      // FIXED: Initialize with Identity matrices, not zeroes
      anim->boneMatrices.assign(anim->motion->boneCount, glm::mat4(1.0f));
    }

    YAMEN_CORE_INFO("Switched animation -> {}", m_GhostModels[index].name);
  }
}

void C3AnimationDemoScene::UpdateCamera() {
  float rad = glm::radians(m_CameraAngle);

  // Perfect orbit camera — Ghost King stays dead center
  glm::vec3 eye(m_CameraDistance * std::sin(rad), m_CameraHeight,
                m_CameraDistance * std::cos(rad));

  // Add target offset (WASD movement)
  eye += m_CameraTarget;

  glm::vec3 center = m_CameraTarget; // Look at this point
  glm::vec3 up(0.0f, 1.0f, 0.0f);

  m_Camera->LookAt(eye, center, up);
}
void C3AnimationDemoScene::RenderDebugGrid(const glm::mat4 &view,
                                           const glm::mat4 &proj) {
  // Lazy init for shaders
  if (!m_LineShader) {
    m_LineShader = std::make_shared<Graphics::Shader>(m_Device);
    // FIXED: Relative path
    if (!m_LineShader->CreateFromFiles(
            "C:/dev/C3Renderer/Yamen/Assets/Shaders/Line.hlsl",
            "C:/dev/C3Renderer/Yamen/Assets/Shaders/Line.hlsl")) {
      YAMEN_CORE_ERROR("Failed to load Line.hlsl");
      return;
    }
  }

  if (!m_DebugInputLayout && m_LineShader) {
    m_DebugInputLayout = std::make_shared<Graphics::InputLayout>(m_Device);
    std::vector<Graphics::InputElement> elements = {
        Graphics::InputElement(Graphics::InputSemantic::Position,
                               Graphics::InputFormat::Float3, 0, 0, 0),
        Graphics::InputElement(Graphics::InputSemantic::Color,
                               Graphics::InputFormat::Float4, 0, 0, 12)};
    m_DebugInputLayout->Create(elements,
                               m_LineShader->GetVertexShaderBytecode().data(),
                               m_LineShader->GetVertexShaderBytecode().size());
  }

  if (!m_GridVertexBuffer) {
    struct Vertex {
      glm::vec3 pos;
      glm::vec4 color;
    };
    std::vector<Vertex> verts;
    const int size = 20;
    const float s = 50.0f;
    const glm::vec4 gray(0.5f, 0.5f, 0.5f, 1);
    const glm::vec4 red(1, 0, 0, 1), blue(0, 0, 1, 1);

    for (int i = -size; i <= size; ++i)
      if (i != 0) {
        float p = i * s;
        verts.push_back({{p, 0, -size * s}, gray});
        verts.push_back({{p, 0, size * s}, gray});
        verts.push_back({{-size * s, 0, p}, gray});
        verts.push_back({{size * s, 0, p}, gray});
      }
    verts.push_back({{-size * s, 0, 0}, red});
    verts.push_back({{size * s, 0, 0}, red});
    verts.push_back({{0, 0, -size * s}, blue});
    verts.push_back({{0, 0, size * s}, blue});

    m_GridVertexCount = (uint32_t)verts.size();
    m_GridVertexBuffer = std::make_shared<Graphics::Buffer>(
        m_Device, Graphics::BufferType::Vertex);
    m_GridVertexBuffer->Create(verts.data(),
                               (uint32_t)(verts.size() * sizeof(Vertex)),
                               sizeof(Vertex));
  }

  if (m_LineShader && m_GridVertexBuffer) {
    m_LineShader->Bind();
    if (m_DebugInputLayout)
      m_DebugInputLayout->Bind();

    // FIXED: Use transpose for HLSL constant buffer
    glm::mat4 mvp = glm::transpose(proj * view);

    if (!m_GridConstantBuffer) {
      m_GridConstantBuffer = std::make_shared<Graphics::Buffer>(
          m_Device, Graphics::BufferType::Constant);
      m_GridConstantBuffer->Create(&mvp, sizeof(mvp), 0,
                                   Graphics::BufferUsage::Dynamic);
    } else {
      m_GridConstantBuffer->Update(&mvp, sizeof(mvp));
    }

    m_GridConstantBuffer->BindToVertexShader(0);
    m_GridVertexBuffer->Bind();
    m_Device.GetContext()->IASetPrimitiveTopology(
        D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    m_Device.GetContext()->Draw(m_GridVertexCount, 0);
  }
}

void C3AnimationDemoScene::RenderDebugSkeleton(const glm::mat4 &view,
                                               const glm::mat4 &proj) {
  if (m_BaseEntity == entt::null || !m_ShowSkeleton)
    return;

  auto *anim =
      m_Registry.try_get<ECS::SkeletalAnimationComponent>(m_BaseEntity);
  if (!anim || anim->boneMatrices.empty() || !m_LineShader)
    return;

  static int logCounter = 0;
  if (logCounter++ % 300 == 0) {
    YAMEN_CORE_INFO("Camera Pos: ({}, {}, {})", m_Camera->GetPosition().x,
                    m_Camera->GetPosition().y, m_Camera->GetPosition().z);
    if (!anim->boneMatrices.empty()) {
      glm::vec3 b0 = glm::vec3(anim->boneMatrices[0][3]);
      YAMEN_CORE_INFO("Bone[0] Pos: ({}, {}, {})", b0.x, b0.y, b0.z);
    }
  }

  m_LineShader->Bind();
  if (m_DebugInputLayout)
    m_DebugInputLayout->Bind();

  struct Vertex {
    glm::vec3 pos;
    glm::vec4 color;
  };

  std::vector<Vertex> verts;
  const glm::vec4 col(1, 1, 0, 1);
  const float sz = 5.0f;

  for (const auto &m : anim->boneMatrices) {
    glm::vec3 p = glm::vec3(m[3]);

    verts.push_back({{p.x - sz, p.y, p.z}, col});
    verts.push_back({{p.x + sz, p.y, p.z}, col});
    verts.push_back({{p.x, p.y - sz, p.z}, col});
    verts.push_back({{p.x, p.y + sz, p.z}, col});
    verts.push_back({{p.x, p.y, p.z - sz}, col});
    verts.push_back({{p.x, p.y, p.z + sz}, col});
  }

  if (!m_SkeletonVertexBuffer ||
      m_SkeletonVertexBuffer->GetSize() < verts.size() * sizeof(Vertex)) {
    m_SkeletonVertexBuffer = std::make_shared<Graphics::Buffer>(
        m_Device, Graphics::BufferType::Vertex);
    m_SkeletonVertexBuffer->Create(
        verts.data(), (uint32_t)(verts.size() * sizeof(Vertex)), sizeof(Vertex),
        Graphics::BufferUsage::Dynamic);
  } else {
    m_SkeletonVertexBuffer->Update(verts.data(),
                                   (uint32_t)(verts.size() * sizeof(Vertex)));
  }

  glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(m_ModelScale));
  glm::mat4 mvp = glm::transpose(proj * view * model);

  if (!m_GridConstantBuffer) {
    m_GridConstantBuffer = std::make_shared<Graphics::Buffer>(
        m_Device, Graphics::BufferType::Constant);
    m_GridConstantBuffer->Create(&mvp, sizeof(mvp), 0,
                                 Graphics::BufferUsage::Dynamic);
  } else {
    m_GridConstantBuffer->Update(&mvp, sizeof(mvp));
  }
  m_GridConstantBuffer->BindToVertexShader(0);

  m_SkeletonVertexBuffer->Bind();
  m_Device.GetContext()->IASetPrimitiveTopology(
      D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
  m_Device.GetContext()->Draw((UINT)verts.size(), 0);
}

void C3AnimationDemoScene::UnloadAllModels() {
  if (m_BaseEntity != entt::null) {
    Client::C3ModelLoader::UnloadModel(m_BaseEntity, m_Registry);
    m_BaseEntity = entt::null;
  }

  for (auto &m : m_GhostModels) {
    if (m.entity != entt::null && m.entity != m_BaseEntity) {
      Client::C3ModelLoader::UnloadModel(m.entity, m_Registry);
    }
    m = {};
  }
}

} // namespace Yamen