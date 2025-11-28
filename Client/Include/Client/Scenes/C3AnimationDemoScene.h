#pragma once

#include "Client/C3ModelLoader.h"
#include "Client/IScene.h"
#include "ECS/Systems/SkeletalAnimationSystem.h"
#include "Graphics/Renderer/C3SkeletalRenderer.h"
#include "Graphics/Renderer/Camera3D.h"
#include <entt/entt.hpp>
#include <string>
#include <vector>

namespace Yamen {

/**
 * @brief Demo scene showcasing C3 skeletal animation
 *
 * Loads and displays animated ghost models from Assets/C3/ghost/085/
 * Demonstrates:
 * - Loading multiple C3 files
 * - Playing different animations
 * - Switching between animation states
 * - Camera controls
 */
class C3AnimationDemoScene : public IScene {
public:
  C3AnimationDemoScene(Graphics::GraphicsDevice &device);
  ~C3AnimationDemoScene() override;

  bool Initialize() override;
  void Update(float deltaTime) override;
  void Render() override;
  void RenderImGui() override;
  const char *GetName() const override { return "C3 Animation Demo"; }

private:
  // Graphics
  Graphics::GraphicsDevice &m_Device;

  // ECS
  entt::registry m_Registry;

  // Rendering
  std::unique_ptr<Graphics::C3SkeletalRenderer> m_SkeletalRenderer;
  std::unique_ptr<Graphics::Camera3D> m_Camera;

  // Ghost models (different animation states)
  struct GhostModel {
    entt::entity entity;
    std::string name;
    std::string filepath;
    bool isLoaded;
    Assets::C3Motion *motion = nullptr; // Pointer to motion data
  };

  std::vector<GhostModel> m_GhostModels;
  int m_CurrentModelIndex;
  entt::entity m_BaseEntity = entt::null; // Entity holding the visible mesh

  // Camera control
  float m_CameraDistance;
  float m_CameraAngle;
  float m_CameraHeight;

  // Animation control
  bool m_AnimationPaused;
  float m_AnimationSpeed;
  float m_ModelScale;

  // Helper functions
  void LoadAllModels();
  void UnloadAllModels();
  void SwitchToModel(int index);
  void UpdateCamera();
};

} // namespace Yamen
