#pragma once

#include "Client/IScene.h"
#include "Core/Math/Math.h"
#include "ECS/Scene.h"
#include "Graphics/Material/Material.h"
#include "Graphics/Mesh/Mesh.h"
#include "Graphics/RHI/GraphicsDevice.h"
#include "Graphics/Renderer/Renderer2D.h"
#include "Graphics/Renderer/Renderer3D.h"
#include "Graphics/Shader/Shader.h"
#include <entt/entt.hpp>
#include <memory>

namespace Yamen {

using namespace Yamen::Core;

/**
 * @brief XPBD Test Scene - Comprehensive physics demonstration
 *
 * Features:
 * - Rigid body tests (stacking, dominos, Newton's cradle)
 * - Constraint tests (distance, joints, chains)
 * - Soft body tests (deformable objects)
 * - Cloth tests (flags, curtains)
 * - Performance comparison (XPBD vs Legacy)
 * - Interactive parameter tuning
 */
class XPBDTestScene : public IScene {
public:
  XPBDTestScene(Graphics::GraphicsDevice &device);
  ~XPBDTestScene() override;

  bool Initialize() override;
  void Update(float deltaTime) override;
  void Render() override;
  void RenderImGui() override;

  const char *GetName() const override { return "XPBD Test Scene"; }

private:
  // Test scenarios
  void CreateRigidBodyTests();
  void CreateConstraintTests();
  void CreateSoftBodyTests();
  void CreateClothTests();
  void CreateStressTest();

  // Specific test setups
  void CreateStackingTest();
  void CreateDominoTest();
  void CreateNewtonsCradle();
  void CreateRopeTest();
  void CreateChainTest();
  void CreateRagdollTest();
  void CreateClothFlag();
  void CreateSoftCube();

  // Helper functions
  entt::entity CreateXPBDBox(const vec3 &position, const vec3 &size, float mass,
                             const vec4 &color);
  entt::entity CreateXPBDSphere(const vec3 &position, float radius, float mass,
                                const vec4 &color);
  void CreateDistanceConstraint(entt::entity a, entt::entity b,
                                float compliance = 0.0f);
  void CreateRope(const vec3 &start, const vec3 &end, int segments,
                  float compliance = 0.001f);

  Graphics::GraphicsDevice &m_Device;

  std::unique_ptr<ECS::Scene> m_Scene;
  std::unique_ptr<Graphics::Renderer3D> m_Renderer3D;
  std::unique_ptr<Graphics::Renderer2D> m_Renderer2D;

  std::shared_ptr<Graphics::Mesh> m_CubeMesh;
  std::shared_ptr<Graphics::Mesh> m_SphereMesh;
  std::unique_ptr<Graphics::Shader> m_Shader;
  std::shared_ptr<Graphics::Texture2D> m_WhiteTexture;

  // Test configuration
  enum class TestMode {
    RigidBodies,
    Constraints,
    SoftBodies,
    Cloth,
    StressTest,
    All
  };
  TestMode m_CurrentTest = TestMode::All;

  bool m_ShowDebugInfo = true;
  bool m_PauseSimulation = false;
  float m_TimeScale = 1.0f;
};

} // namespace Yamen
