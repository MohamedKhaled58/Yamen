#include "Client/XPBDTestScene.h"
#include "Client/CameraController.h"
#include "ECS/Components.h"
#include "ECS/Components/XPBDComponents.h"
#include "ECS/Physics/PhysicsMaterial.h"
#include "ECS/Systems/CameraSystem.h"
#include "ECS/Systems/RenderSystem.h"
#include "ECS/Systems/ScriptSystem.h"
#include "ECS/Systems/XPBDSolver.h"
#include "Graphics/Mesh/MeshBuilder.h"
#include <Core/Logging/Logger.h>
#include <Core/Math/Math.h>
#include <imgui.h>
#include <random>

namespace Yamen {

using namespace Core::Math;

XPBDTestScene::XPBDTestScene(Graphics::GraphicsDevice &device)
    : m_Device(device) {}

XPBDTestScene::~XPBDTestScene() {}

bool XPBDTestScene::Initialize() {
  m_Scene = std::make_unique<ECS::Scene>("XPBD Test Scene");

  m_Renderer3D = std::make_unique<Graphics::Renderer3D>(m_Device);
  if (!m_Renderer3D->Initialize()) {
    YAMEN_CORE_ERROR("Failed to initialize Renderer3D");
    return false;
  }

  m_Renderer2D = std::make_unique<Graphics::Renderer2D>(m_Device);
  if (!m_Renderer2D->Initialize()) {
    YAMEN_CORE_ERROR("Failed to initialize Renderer2D");
    return false;
  }

  // Add systems
  m_Scene->AddSystem<ECS::CameraSystem>();
  m_Scene->AddSystem<ECS::ScriptSystem>();
  m_Scene->AddSystem<ECS::XPBDSolver>(); // Use XPBD solver instead of legacy
  m_Scene->AddSystem<ECS::RenderSystem>(m_Device, m_Renderer3D.get(),
                                        m_Renderer2D.get());
  m_Scene->OnInit();

  // Create meshes
  std::vector<Graphics::Vertex3D> vertices;
  std::vector<uint32_t> indices;

  Graphics::MeshBuilder::CreateCube(vertices, indices, 1.0f);
  m_CubeMesh = std::make_shared<Graphics::Mesh>(m_Device);
  m_CubeMesh->Create(vertices, indices);

  Graphics::MeshBuilder::CreateSphere(vertices, indices, 0.5f, 16, 16);
  m_SphereMesh = std::make_shared<Graphics::Mesh>(m_Device);
  m_SphereMesh->Create(vertices, indices);

  m_Shader = std::make_unique<Graphics::Shader>(m_Device);
  m_Shader->CreateFromFiles(
      "C:/dev/C3Renderer/Yamen/Assets/Shaders/Basic3D.hlsl",
      "C:/dev/C3Renderer/Yamen/Assets/Shaders/Basic3D.hlsl", "VSMain",
      "PSMain");

  m_WhiteTexture = std::make_shared<Graphics::Texture2D>(m_Device);
  uint32_t whitePixel = 0xFFFFFFFF;
  m_WhiteTexture->Create(1, 1, Graphics::TextureFormat::R8G8B8A8_UNORM,
                         &whitePixel);

  // Create camera
  auto cameraEntity = m_Scene->CreateEntity("MainCamera");
  auto &cameraComp = cameraEntity.AddComponent<ECS::CameraComponent>();
  cameraComp.Primary = true;
  cameraComp.Camera.SetFOV(60.0f);
  cameraComp.Camera.SetAspectRatio(16.0f / 9.0f);
  cameraComp.Camera.SetClipPlanes(0.1f, 1000.0f);

  auto &cameraTransform = cameraEntity.GetComponent<ECS::TransformComponent>();
  cameraTransform.Translation =
      vec3(5.0f, 3.0f, 5.0f); // Closer and angled view
  cameraTransform.Rotation =
      quat(vec3(Math::Radians(-20.0f), Math::Radians(-45.0f), 0.0f));

  // Add camera controller for movement
  auto &script = cameraEntity.AddComponent<ECS::NativeScriptComponent>();
  script.Bind<Client::CameraController>();

  // Create light - much brighter
  auto lightEntity = m_Scene->CreateEntity("Sun");
  auto &lightComp = lightEntity.AddComponent<ECS::LightComponent>();
  lightComp.LightData = Graphics::Light::CreateDirectional(
      Math::Normalize(vec3(-0.3f, -1.0f, -0.3f)), // From above
      vec3(1.0f, 1.0f, 1.0f),                     // White light
      5.0f                                        // Much brighter intensity
  );
  lightComp.Active = true;

  // Create ground - thicker and at y=0
  auto ground = CreateXPBDBox(vec3(0, -2, 0), vec3(50, 2, 50), 0.0f,
                              vec4(0.5f, 0.5f, 0.5f, 1.0f));

  // Create test scenarios based on mode
  // TEMPORARY: Only create minimal test for debugging
  CreateStackingTest();

  /* Disabled for performance testing
  if (m_CurrentTest == TestMode::All ||
      m_CurrentTest == TestMode::RigidBodies) {
    CreateRigidBodyTests();
  }
  if (m_CurrentTest == TestMode::All ||
      m_CurrentTest == TestMode::Constraints) {
    CreateConstraintTests();
  }
  if (m_CurrentTest == TestMode::All || m_CurrentTest == TestMode::SoftBodies) {
    CreateSoftBodyTests();
  }
  if (m_CurrentTest == TestMode::All || m_CurrentTest == TestMode::Cloth) {
    CreateClothTests();
  }
  if (m_CurrentTest == TestMode::StressTest) {
    CreateStressTest();
  }
  */

  YAMEN_CORE_INFO("XPBD Test Scene Initialized");
  return true;
}

void XPBDTestScene::CreateRigidBodyTests() {
  CreateStackingTest();
  CreateDominoTest();
  CreateNewtonsCradle();
}

void XPBDTestScene::CreateConstraintTests() {
  CreateRopeTest();
  CreateChainTest();
}

void XPBDTestScene::CreateSoftBodyTests() { CreateSoftCube(); }

void XPBDTestScene::CreateClothTests() { CreateClothFlag(); }

void XPBDTestScene::CreateStressTest() {
  // Create 500 boxes in a grid
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> colorDist(0.3f, 1.0f);

  for (int i = 0; i < 500; ++i) {
    float x = (i % 25) * 2.0f - 25.0f;
    float z = (i / 25) * 2.0f - 25.0f;
    float y = 20.0f + (i % 5) * 2.0f;

    vec4 color(colorDist(gen), colorDist(gen), colorDist(gen), 1.0f);
    CreateXPBDBox(vec3(x, y, z), vec3(0.8f), 1.0f, color);
  }
}

void XPBDTestScene::CreateStackingTest() {
  // Minimal test: 3 bigger boxes stacked higher, starting above ground at y=0
  CreateXPBDBox(vec3(0, 2.0f, 0), vec3(1.0f), 1.0f, vec4(1, 0, 0, 1)); // Red
  CreateXPBDBox(vec3(0, 4.0f, 0), vec3(1.0f), 1.0f, vec4(0, 1, 0, 1)); // Green
  CreateXPBDBox(vec3(0, 6.0f, 0), vec3(1.0f), 1.0f, vec4(0, 0, 1, 1)); // Blue
}

void XPBDTestScene::CreateDominoTest() {
  // Create domino chain
  int dominoCount = 15;
  float spacing = 1.5f;
  float startX = 10.0f;
  float startZ = -10.0f;

  for (int i = 0; i < dominoCount; ++i) {
    vec3 pos(startX, 1.5f, startZ + i * spacing);
    auto domino = CreateXPBDBox(pos, vec3(0.1f, 1.5f, 0.5f), 0.5f,
                                vec4(0.8f, 0.2f, 0.2f, 1.0f));
  }

  // Create a ball to knock them down
  auto ball = CreateXPBDSphere(vec3(startX - 5.0f, 1.0f, startZ), 0.5f, 2.0f,
                               vec4(0.2f, 0.8f, 0.2f, 1.0f));
  auto &particle = m_Scene->Registry().get<ECS::XPBDParticleComponent>(ball);
  particle.Velocity = vec3(8.0f, 0.0f, 0.0f);
}

void XPBDTestScene::CreateNewtonsCradle() {
  // Create Newton's cradle with 5 spheres
  int sphereCount = 5;
  float spacing = 1.1f;
  float ropeLength = 5.0f;
  vec3 anchorStart(-5.0f, 10.0f, 10.0f);

  for (int i = 0; i < sphereCount; ++i) {
    vec3 anchorPos = anchorStart + vec3(i * spacing, 0, 0);
    vec3 spherePos = anchorPos + vec3(0, -ropeLength, 0);

    // Create anchor (static)
    auto anchor = CreateXPBDBox(anchorPos, vec3(0.1f), 0.0f,
                                vec4(0.5f, 0.5f, 0.5f, 1.0f));

    // Create sphere
    auto sphere =
        CreateXPBDSphere(spherePos, 0.5f, 1.0f, vec4(0.3f, 0.3f, 0.8f, 1.0f));

    // Create distance constraint (rope)
    CreateDistanceConstraint(anchor, sphere, 0.0f);
  }

  // Pull first sphere back
  auto firstSphere =
      m_Scene->Registry().view<ECS::XPBDParticleComponent>().back();
  auto &particle =
      m_Scene->Registry().get<ECS::XPBDParticleComponent>(firstSphere);
  particle.Position.x -= 3.0f;
  particle.Position.y += 2.0f;
}

void XPBDTestScene::CreateRopeTest() {
  // Create a rope bridge
  vec3 start(-15.0f, 8.0f, -10.0f);
  vec3 end(-5.0f, 8.0f, -10.0f);
  CreateRope(start, end, 20, 0.001f);
}

void XPBDTestScene::CreateChainTest() {
  // Create a hanging chain
  vec3 start(0.0f, 15.0f, -10.0f);
  vec3 end(0.0f, 5.0f, -10.0f);
  CreateRope(start, end, 15, 0.0001f);
}

void XPBDTestScene::CreateRagdollTest() {
  // TODO: Implement ragdoll with ball-socket joints
}

void XPBDTestScene::CreateClothFlag() {
  // Create a cloth flag using distance and bending constraints
  // Reduced resolution for performance with O(N²) collision detection
  int width = 10; // Was 20
  int height = 8; // Was 15
  float spacing = 0.3f;
  vec3 startPos(5.0f, 10.0f, 5.0f);

  std::vector<entt::entity> particles;
  particles.resize(width * height);

  // Create particle grid
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int idx = y * width + x;
      vec3 pos = startPos + vec3(x * spacing, -y * spacing, 0);

      float mass = (y == 0) ? 0.0f : 0.1f; // Top row is fixed
      particles[idx] =
          CreateXPBDBox(pos, vec3(0.1f), mass, vec4(0.9f, 0.9f, 0.9f, 1.0f));
    }
  }

  // Create structural constraints (horizontal and vertical)
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int idx = y * width + x;

      // Horizontal constraint
      if (x < width - 1) {
        CreateDistanceConstraint(particles[idx], particles[idx + 1], 0.01f);
      }

      // Vertical constraint
      if (y < height - 1) {
        CreateDistanceConstraint(particles[idx], particles[idx + width], 0.01f);
      }
    }
  }

  // Create bending constraints (skip one particle)
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width - 2; ++x) {
      int idx = y * width + x;
      CreateDistanceConstraint(particles[idx], particles[idx + 2], 0.1f);
    }
  }
  for (int y = 0; y < height - 2; ++y) {
    for (int x = 0; x < width; ++x) {
      int idx = y * width + x;
      CreateDistanceConstraint(particles[idx], particles[idx + width * 2],
                               0.1f);
    }
  }
}

void XPBDTestScene::CreateSoftCube() {
  // Create a soft body cube using volume constraints
  vec3 center(0.0f, 5.0f, 5.0f);
  float size = 2.0f;

  // Create 8 corner particles
  std::vector<entt::entity> corners;
  for (int z = 0; z < 2; ++z) {
    for (int y = 0; y < 2; ++y) {
      for (int x = 0; x < 2; ++x) {
        vec3 offset(x - 0.5f, y - 0.5f, z - 0.5f);
        vec3 pos = center + offset * size;
        corners.push_back(
            CreateXPBDSphere(pos, 0.2f, 0.5f, vec4(0.8f, 0.4f, 0.8f, 1.0f)));
      }
    }
  }

  // Create distance constraints for edges
  int edges[][2] = {
      {0, 1}, {1, 3}, {3, 2}, {2, 0}, // Bottom face
      {4, 5}, {5, 7}, {7, 6}, {6, 4}, // Top face
      {0, 4}, {1, 5}, {2, 6}, {3, 7}  // Vertical edges
  };

  for (auto &edge : edges) {
    CreateDistanceConstraint(corners[edge[0]], corners[edge[1]], 0.05f);
  }

  // Create diagonal constraints for rigidity
  CreateDistanceConstraint(corners[0], corners[7], 0.05f);
  CreateDistanceConstraint(corners[1], corners[6], 0.05f);
  CreateDistanceConstraint(corners[2], corners[5], 0.05f);
  CreateDistanceConstraint(corners[3], corners[4], 0.05f);
}

entt::entity XPBDTestScene::CreateXPBDBox(const vec3 &position,
                                          const vec3 &size, float mass,
                                          const vec4 &color) {
  auto entity = m_Scene->CreateEntity("XPBDBox");

  // Add mesh component
  auto &mesh = entity.AddComponent<ECS::MeshComponent>();
  mesh.Mesh = m_CubeMesh;

  auto material = std::make_shared<Graphics::Material>();
  material->SetShader(m_Shader.get());
  material->SetTexture(Graphics::Material::DIFFUSE_TEXTURE,
                       m_WhiteTexture.get());
  material->SetVector(Graphics::Material::ALBEDO_COLOR, color);
  mesh.Material = material;

  // Set transform
  auto &transform = entity.GetComponent<ECS::TransformComponent>();
  transform.Translation = position;
  transform.Scale = size;

  // Add XPBD particle
  auto &particle = entity.AddComponent<ECS::XPBDParticleComponent>();
  particle.Position = position;
  particle.PreviousPosition = position;
  particle.SetMass(mass);

  // Add collider
  ECS::BoxCollider collider;
  collider.HalfExtents = size * 0.5f;
  entity.AddComponent<ECS::ColliderComponent>(collider);

  return entity;
}

entt::entity XPBDTestScene::CreateXPBDSphere(const vec3 &position, float radius,
                                             float mass, const vec4 &color) {
  auto entity = m_Scene->CreateEntity("XPBDSphere");

  // Add mesh component
  auto &mesh = entity.AddComponent<ECS::MeshComponent>();
  mesh.Mesh = m_SphereMesh;

  auto material = std::make_shared<Graphics::Material>();
  material->SetShader(m_Shader.get());
  material->SetTexture(Graphics::Material::DIFFUSE_TEXTURE,
                       m_WhiteTexture.get());
  material->SetVector(Graphics::Material::ALBEDO_COLOR, color);
  mesh.Material = material;

  // Set transform
  auto &transform = entity.GetComponent<ECS::TransformComponent>();
  transform.Translation = position;
  transform.Scale = vec3(radius * 2.0f);

  // Add XPBD particle
  auto &particle = entity.AddComponent<ECS::XPBDParticleComponent>();
  particle.Position = position;
  particle.PreviousPosition = position;
  particle.SetMass(mass);

  // Add collider
  ECS::SphereCollider collider;
  collider.Radius = radius;
  entity.AddComponent<ECS::ColliderComponent>(collider);

  return entity;
}

void XPBDTestScene::CreateDistanceConstraint(entt::entity a, entt::entity b,
                                             float compliance) {
  auto &registry = m_Scene->Registry();

  auto *pA = registry.try_get<ECS::XPBDParticleComponent>(a);
  auto *pB = registry.try_get<ECS::XPBDParticleComponent>(b);

  if (!pA || !pB)
    return;

  float restLength = Math::Length(pA->Position - pB->Position);

  auto constraintEntity = m_Scene->CreateEntity("DistanceConstraint");
  auto &constraint =
      constraintEntity.AddComponent<ECS::XPBDConstraintComponent>();
  constraint.Constraint = ECS::DistanceConstraint(a, b, restLength, compliance);
}

void XPBDTestScene::CreateRope(const vec3 &start, const vec3 &end, int segments,
                               float compliance) {
  std::vector<entt::entity> particles;

  for (int i = 0; i <= segments; ++i) {
    float t = (float)i / segments;
    vec3 pos = Math::Lerp(start, end, t);

    float mass = (i == 0) ? 0.0f : 0.2f; // First particle is fixed
    particles.push_back(
        CreateXPBDSphere(pos, 0.15f, mass, vec4(0.6f, 0.4f, 0.2f, 1.0f)));
  }

  // Connect with distance constraints
  for (int i = 0; i < segments; ++i) {
    CreateDistanceConstraint(particles[i], particles[i + 1], compliance);
  }
}

void XPBDTestScene::Update(float deltaTime) {
  if (m_PauseSimulation)
    return;

  if (m_Scene) {
    m_Scene->OnUpdate(deltaTime * m_TimeScale);
  }
}

void XPBDTestScene::Render() {
  if (m_Scene) {
    m_Scene->OnRender();
  }
}

void XPBDTestScene::RenderImGui() {
  if (!m_Scene)
    return;

  ImGui::Begin("XPBD Test Scene Controls");

  ImGui::Text("XPBD Physics Engine Demo");
  ImGui::Separator();

  // Test mode selection
  const char *testModes[] = {"Rigid Bodies", "Constraints", "Soft Bodies",
                             "Cloth",        "Stress Test", "All"};
  int currentMode = static_cast<int>(m_CurrentTest);
  if (ImGui::Combo("Test Mode", &currentMode, testModes,
                   IM_ARRAYSIZE(testModes))) {
    m_CurrentTest = static_cast<TestMode>(currentMode);
  }

  ImGui::Separator();

  // Simulation controls
  ImGui::Checkbox("Pause Simulation", &m_PauseSimulation);
  ImGui::SliderFloat("Time Scale", &m_TimeScale, 0.0f, 2.0f);

  if (ImGui::Button("Reset Scene")) {
    // TODO: Implement scene reset
  }

  ImGui::Separator();

  // XPBD solver settings
  auto *solver = m_Scene->GetSystem<ECS::XPBDSolver>();
  if (solver) {
    ImGui::Text("XPBD Solver Settings");
    ImGui::DragFloat3("Gravity", &solver->Gravity.x, 0.1f, -50.0f, 50.0f);
    ImGui::DragInt("SubSteps", &solver->SubSteps, 1, 1, 20);
    ImGui::DragInt("Solver Iterations", &solver->SolverIterations, 1, 1, 50);
    ImGui::Checkbox("Enable Sleeping", &solver->EnableSleeping);
    ImGui::Checkbox("Enable Warm Starting", &solver->EnableWarmStarting);

    ImGui::Separator();

    // Statistics
    auto stats = solver->GetStats();
    ImGui::Text("Statistics");
    ImGui::Text("Active Particles: %d", stats.ActiveParticles);
    ImGui::Text("Sleeping Particles: %d", stats.SleepingParticles);
    ImGui::Text("Active Constraints: %d", stats.ActiveConstraints);
    ImGui::Text("Contact Constraints: %d", stats.ContactConstraints);
    ImGui::Text("Solve Time: %.2f ms", stats.SolveTime);
    ImGui::Text("Collision Time: %.2f ms", stats.CollisionTime);
  }

  ImGui::Separator();
  ImGui::Checkbox("Show Debug Info", &m_ShowDebugInfo);

  ImGui::End();
}

} // namespace Yamen
