#pragma once

#include "Client/C3ModelLoader.h"
#include "Client/IScene.h"
#include "Core/Math/Math.h"
#include "ECS/Systems/SkeletalAnimationSystem.h"
#include "Graphics/RHI/Buffer.h"
#include "Graphics/RHI/InputLayout.h"
#include "Graphics/Renderer/C3SkeletalRenderer.h"
#include "Graphics/Renderer/Camera3D.h"
#include "Graphics/Shader/Shader.h"


#include <entt/entt.hpp>
#include <memory>
#include <string>
#include <vector>

namespace Yamen {

using namespace Yamen::Core;

struct GhostModelEntry {
  entt::entity entity = entt::null;
  std::string name;
  std::string filepath;
  bool isLoaded = false;
  Assets::C3Motion *motion = nullptr;
};

class C3AnimationDemoScene : public IScene {
public:
  explicit C3AnimationDemoScene(Graphics::GraphicsDevice &device);
  ~C3AnimationDemoScene() override;

  bool Initialize() override;
  void Update(float deltaTime) override;
  void Render() override;
  void RenderImGui() override;
  void UnloadAllModels();

  const char *GetName() const override {
    return "C3 Animation Demo - Ghost King";
  }

private:
  void LoadAllModels();
  void SwitchToModel(int index);
  void UpdateCamera();
  void RenderDebugGrid(const mat4 &view, const mat4 &proj);
  void RenderDebugSkeleton(const mat4 &view, const mat4 &proj);
  void SetModel(size_t index);
  void CalculateInverseBindMatrices();


    Graphics::GraphicsDevice &m_Device;
    entt::registry m_Registry;

    std::unique_ptr<Graphics::Camera3D> m_Camera;
    std::unique_ptr<Graphics::C3SkeletalRenderer> m_SkeletalRenderer;

    std::vector<GhostModelEntry> m_GhostModels;
    entt::entity m_BaseEntity = entt::null;
    int m_CurrentModelIndex = 0;

    // Camera
    float m_CameraDistance = 1000.0f;
    float m_CameraAngle = 0.0f;
    float m_CameraHeight = 450.0f;
    vec3 m_CameraTarget = vec3(0.0f);

    // Animation
    bool m_AnimationPaused = false;
    float m_AnimationSpeed = 30.0f;

    // Model
    float m_ModelScale = 1.0f;
    bool m_ShowSkeleton = true;

    // Debug rendering
    std::shared_ptr<Graphics::Shader> m_LineShader;
    std::shared_ptr<Graphics::InputLayout> m_DebugInputLayout;
    std::shared_ptr<Graphics::Buffer> m_GridVertexBuffer;
    std::shared_ptr<Graphics::Buffer> m_GridConstantBuffer;
    std::shared_ptr<Graphics::Buffer> m_SkeletonConstantBuffer;
    std::shared_ptr<Graphics::Buffer> m_SkeletonVertexBuffer;
    uint32_t m_GridVertexCount = 0;
  };

} // namespace Yamen