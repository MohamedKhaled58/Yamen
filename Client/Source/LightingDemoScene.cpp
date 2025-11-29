#include "Client/LightingDemoScene.h"
#include "Client/CameraController.h"
#include "ECS/Components.h"
#include "ECS/Systems/CameraSystem.h"
#include "ECS/Systems/PhysicsSystem.h"
#include "ECS/Systems/RenderSystem.h"
#include "ECS/Systems/ScriptSystem.h"
#include "Graphics/Mesh/MeshBuilder.h"
#include <Core/Logging/Logger.h>
#include <Core/Math/Math.h>
#include <imgui.h>

using namespace Yamen::Core;

namespace Yamen {

using namespace Core::Math;

LightingDemoScene::LightingDemoScene(Graphics::GraphicsDevice &device)
    : m_Device(device) {}

LightingDemoScene::~LightingDemoScene() {}

bool LightingDemoScene::Initialize() {
  m_Scene = std::make_unique<ECS::Scene>("Lighting Demo");

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
  m_Scene->AddSystem<ECS::PhysicsSystem>();
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
  cameraTransform.Translation = vec3(0.0f, 5.0f, -15.0f);
  // Rotate 90 degrees around Y to face +Z (since 0 degrees is +X in
  // CameraController)
  cameraTransform.Rotation = quat(vec3(0.0f, Math::Radians(90.0f), 0.0f));

  auto &script = cameraEntity.AddComponent<ECS::NativeScriptComponent>();
  script.Bind<Client::CameraController>();

  // Create scene
  CreateEnvironment();
  CreateDirectionalLight();
  CreatePointLights();
  CreateSpotLight();

  YAMEN_CORE_INFO("Lighting Demo Scene Initialized");
  return true;
}

void LightingDemoScene::CreateEnvironment() {
  // Ground
  auto ground = m_Scene->CreateEntity("Ground");
  auto &mesh = ground.AddComponent<ECS::MeshComponent>();
  mesh.Mesh = m_CubeMesh;

  auto material = std::make_shared<Graphics::Material>();
  material->SetShader(m_Shader.get());
  material->SetTexture(Graphics::Material::DIFFUSE_TEXTURE,
                       m_WhiteTexture.get());
  material->SetVector(Graphics::Material::ALBEDO_COLOR,
                      vec4(0.2f, 0.3f, 0.2f, 1.0f));
  mesh.Material = material;

  auto &transform = ground.GetComponent<ECS::TransformComponent>();
  transform.Translation = vec3(0.0f, -1.0f, 0.0f);
  transform.Scale = vec3(30.0f, 1.0f, 30.0f);

  // Create pillars
  for (int i = 0; i < 4; ++i) {
    float angle = i * XM_PI * 0.5f;
    float radius = 8.0f;

    auto pillar = m_Scene->CreateEntity("Pillar");
    auto &pillarMesh = pillar.AddComponent<ECS::MeshComponent>();
    pillarMesh.Mesh = m_CubeMesh;

    auto pillarMat = std::make_shared<Graphics::Material>();
    pillarMat->SetShader(m_Shader.get());
    pillarMat->SetTexture(Graphics::Material::DIFFUSE_TEXTURE,
                          m_WhiteTexture.get());
    pillarMat->SetVector(Graphics::Material::ALBEDO_COLOR,
                         vec4(0.7f, 0.7f, 0.7f, 1.0f));
    pillarMesh.Material = pillarMat;

    auto &pillarTransform = pillar.GetComponent<ECS::TransformComponent>();
    pillarTransform.Translation =
        vec3(cos(angle) * radius, 2.5f, sin(angle) * radius);
    pillarTransform.Scale = vec3(1.0f, 5.0f, 1.0f);
  }

  // Center sphere
  auto centerSphere = m_Scene->CreateEntity("CenterSphere");
  auto &sphereMesh = centerSphere.AddComponent<ECS::MeshComponent>();
  sphereMesh.Mesh = m_SphereMesh;

  auto sphereMat = std::make_shared<Graphics::Material>();
  sphereMat->SetShader(m_Shader.get());
  sphereMat->SetTexture(Graphics::Material::DIFFUSE_TEXTURE,
                        m_WhiteTexture.get());
  sphereMat->SetVector(Graphics::Material::ALBEDO_COLOR,
                       vec4(0.9f, 0.9f, 0.9f, 1.0f));
  sphereMesh.Material = sphereMat;

  auto &sphereTransform = centerSphere.GetComponent<ECS::TransformComponent>();
  sphereTransform.Translation = vec3(0.0f, 2.0f, 0.0f);
  sphereTransform.Scale = vec3(2.0f);
}

void LightingDemoScene::CreateDirectionalLight() {
  auto light = m_Scene->CreateEntity("DirectionalLight");
  auto &lightComp = light.AddComponent<ECS::LightComponent>();
  lightComp.LightData = Graphics::Light::CreateDirectional(
      Math::Normalize(vec3(-0.5f, -1.0f, -0.3f)),
      vec3(1.0f, 0.9f, 0.8f), // Warm sunlight
      1.5f);
  lightComp.Active = true;
  m_DirectionalLight = light;
}

void LightingDemoScene::CreatePointLights() {
  // Create orbiting point lights
  vec3 colors[] = {
      vec3(1.0f, 0.2f, 0.2f), // Red
      vec3(0.2f, 1.0f, 0.2f), // Green
      vec3(0.2f, 0.2f, 1.0f), // Blue
      vec3(1.0f, 1.0f, 0.2f)  // Yellow
  };

  for (int i = 0; i < 4; ++i) {
    auto light = m_Scene->CreateEntity("PointLight");
    auto &lightComp = light.AddComponent<ECS::LightComponent>();
    lightComp.LightData =
        Graphics::Light::CreatePoint(vec3(0.0f), colors[i], 3.0f, 10.0f);
    lightComp.Active = true;

    // Add visual sphere for the light
    auto &mesh = light.AddComponent<ECS::MeshComponent>();
    mesh.Mesh = m_SphereMesh;

    auto material = std::make_shared<Graphics::Material>();
    material->SetShader(m_Shader.get());
    material->SetTexture(Graphics::Material::DIFFUSE_TEXTURE,
                         m_WhiteTexture.get());
    material->SetVector(Graphics::Material::ALBEDO_COLOR,
                        vec4(colors[i], 1.0f));
    mesh.Material = material;

    auto &transform = light.GetComponent<ECS::TransformComponent>();
    transform.Scale = vec3(0.3f);

    m_PointLights.push_back(light);
  }
}

void LightingDemoScene::CreateSpotLight() {
  auto light = m_Scene->CreateEntity("SpotLight");
  auto &lightComp = light.AddComponent<ECS::LightComponent>();
  lightComp.LightData = Graphics::Light::CreateSpot(
      vec3(0.0f, 10.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f),
      5.0f, 15.0f, 25.0f);
  lightComp.Active = true;
  m_SpotLight = light;
}

void LightingDemoScene::UpdateLights(float deltaTime) {
  if (!m_AnimateLights)
    return;

  m_TimeOfDay += deltaTime * 0.1f;
  if (m_TimeOfDay > XM_2PI) {
    m_TimeOfDay -= XM_2PI;
  }

  // Update directional light (sun)
  if (m_DirectionalLight != entt::null) {
    auto &lightComp =
        m_Scene->Registry().get<ECS::LightComponent>(m_DirectionalLight);

    // Rotate sun direction
    float sunAngle = m_TimeOfDay;
    vec3 sunDir = Math::Normalize(
        vec3(cos(sunAngle), -abs(sin(sunAngle)), sin(sunAngle) * 0.3f));
    lightComp.LightData.direction = sunDir;

    // Change color based on time of day
    float dayFactor = (sin(sunAngle) + 1.0f) * 0.5f;
    lightComp.LightData.color =
        Math::Lerp(vec3(0.2f, 0.2f, 0.4f), // Night (blue)
                   vec3(1.0f, 0.9f, 0.7f), // Day (warm)
                   dayFactor);
    lightComp.LightData.intensity = 0.5f + dayFactor * 1.5f;
  }

  // Update orbiting point lights
  for (size_t i = 0; i < m_PointLights.size(); ++i) {
    if (m_PointLights[i] == entt::null)
      continue;

    auto &transform =
        m_Scene->Registry().get<ECS::TransformComponent>(m_PointLights[i]);
    auto &lightComp =
        m_Scene->Registry().get<ECS::LightComponent>(m_PointLights[i]);

    float angle = m_TimeOfDay + i * XM_PI * 0.5f;
    float radius = 5.0f;
    float height = 3.0f + sin(angle * 2.0f) * 2.0f;

    transform.Translation =
        vec3(cos(angle) * radius, height, sin(angle) * radius);

    lightComp.LightData.position = transform.Translation;
  }
}

void LightingDemoScene::Update(float deltaTime) {
  if (m_Scene) {
    UpdateLights(deltaTime);
    m_Scene->OnUpdate(deltaTime);
  }
}

void LightingDemoScene::Render() {
  if (m_Scene) {
    m_Scene->OnRender();
  }
}

void LightingDemoScene::RenderImGui() {
  if (!m_Scene)
    return;

  ImGui::Begin("Lighting Demo Controls");

  ImGui::Text("Dynamic Lighting Showcase");
  ImGui::Separator();

  ImGui::Checkbox("Animate Lights", &m_AnimateLights);
  ImGui::SliderFloat("Time of Day", &m_TimeOfDay, 0.0f, XM_2PI);

  ImGui::Separator();
  ImGui::Text("Light Count: %d",
              1 + m_PointLights.size() + 1); // Dir + Points + Spot

  if (ImGui::CollapsingHeader("Directional Light")) {
    if (m_DirectionalLight != entt::null) {
      auto &lightComp =
          m_Scene->Registry().get<ECS::LightComponent>(m_DirectionalLight);
      ImGui::ColorEdit3("Color", &lightComp.LightData.color.x);
      ImGui::DragFloat("Intensity", &lightComp.LightData.intensity, 0.1f, 0.0f,
                       10.0f);
      ImGui::DragFloat3("Direction", &lightComp.LightData.direction.x, 0.01f);
    }
  }

  ImGui::End();
}

} // namespace Yamen
