
#include "Client/DemoScene.h"
#include "Graphics/Mesh/MeshBuilder.h"
#include "Graphics/Texture/TextureLoader.h"
#include <Core/Logging/Logger.h>
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>

namespace Yamen {

    DemoScene::DemoScene(Graphics::GraphicsDevice& device)
        : m_Device(device)
        , m_Rotation(0.0f)
        , m_ShowWireframe(false)
        , m_Show2D(true)
        , m_Show3D(true)
        , m_LightDirection(0.0f, -1.0f, 0.3f)
        , m_LightColor(1.0f, 0.95f, 0.8f)
        , m_UseMaterials(false)
    {
    }

    DemoScene::~DemoScene() {
    }

    bool DemoScene::Initialize() {
        YAMEN_CORE_INFO("=== Initializing Demo Scene ===");

        // Create renderers
        m_Renderer2D = std::make_unique<Graphics::Renderer2D>(m_Device);
        if (!m_Renderer2D->Initialize()) {
            YAMEN_CORE_ERROR("Failed to initialize Renderer2D");
            return false;
        }

        m_Renderer3D = std::make_unique<Graphics::Renderer3D>(m_Device);
        if (!m_Renderer3D->Initialize()) {
            YAMEN_CORE_ERROR("Failed to initialize Renderer3D");
            return false;
        }

        // Create cameras
        m_Camera2D = std::make_unique<Graphics::Camera2D>(1280.0f, 720.0f);
        m_Camera2D->SetPosition(640.0f, 360.0f);

        m_Camera3D = std::make_unique<Graphics::Camera3D>(60.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
        m_Camera3D->SetPosition(0.0f, 5.0f, -15.0f);
        // Yaw 90 degrees to look at +Z (where objects are), Pitch -15 to look slightly down
        m_Camera3D->SetRotation(glm::vec3(glm::radians(-15.0f), glm::radians(90.0f), 0.0f));

        // Create test meshes
        CreateTestMeshes();

        // Create test textures
        CreateTestTextures();

        // Create ShaderLibrary
        m_ShaderLibrary = std::make_unique<Graphics::ShaderLibrary>(m_Device);
        m_ShaderLibrary->PrecompileDefaults(); // Loads C3 shaders

        // Create LightManager
        m_LightManager = std::make_unique<Graphics::LightManager>();

        // Setup lights
        m_SunLight = Graphics::Light::CreateDirectional(glm::vec3(0.3f, -1.0f, 0.2f), glm::vec3(1.0f, 0.95f, 0.8f), 0.8f);
        m_PointLight1 = Graphics::Light::CreatePoint(glm::vec3(-5, 3, 5), glm::vec3(1, 0, 0), 2.0f, 10.0f);
        m_PointLight2 = Graphics::Light::CreatePoint(glm::vec3(5, 3, 5), glm::vec3(0, 0, 1), 2.0f, 10.0f);

        m_LightManager->AddLight(&m_SunLight);
        m_LightManager->AddLight(&m_PointLight1);
        m_LightManager->AddLight(&m_PointLight2);

        // Call CreateTestMaterials
        CreateTestMaterials();

        YAMEN_CORE_INFO("Demo Scene initialized successfully");
        return true;
    }

    void DemoScene::CreateTestMeshes() {
        std::vector<Graphics::Vertex3D> vertices;
        std::vector<uint32_t> indices;

        // Cube
        Graphics::MeshBuilder::CreateCube(vertices, indices, 2.0f);
        m_CubeMesh = std::make_unique<Graphics::Mesh>(m_Device);
        m_CubeMesh->Create(vertices, indices);

        // Sphere
        vertices.clear();
        indices.clear();
        Graphics::MeshBuilder::CreateSphere(vertices, indices, 1.5f, 32, 16);
        m_SphereMesh = std::make_unique<Graphics::Mesh>(m_Device);
        m_SphereMesh->Create(vertices, indices);

        // Plane
        vertices.clear();
        indices.clear();
        Graphics::MeshBuilder::CreatePlane(vertices, indices, 20.0f, 20.0f, 10, 10);
        m_PlaneMesh = std::make_unique<Graphics::Mesh>(m_Device);
        m_PlaneMesh->Create(vertices, indices);

        YAMEN_CORE_INFO("Test meshes created");
    }

    void DemoScene::CreateTestTextures() {
        // Create a simple test texture (white for now)
        m_TestTexture = Graphics::TextureLoader::CreateSolidColor(m_Device, 64, 64, 255, 255, 255, 255);
        YAMEN_CORE_INFO("Test textures created");
    }

    void DemoScene::CreateTestMaterials() {
        auto* shader = m_ShaderLibrary->Get("Basic3D");

        // Red material
        m_RedMaterial = std::make_unique<Graphics::Material>();
        m_RedMaterial->SetShader(shader);
        m_RedMaterial->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_TestTexture.get());
        m_RedMaterial->SetVector(Graphics::Material::ALBEDO_COLOR, glm::vec4(1, 0, 0, 1));

        // Green material
        m_GreenMaterial = std::make_unique<Graphics::Material>();
        m_GreenMaterial->SetShader(shader);
        m_GreenMaterial->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_TestTexture.get());
        m_GreenMaterial->SetVector(Graphics::Material::ALBEDO_COLOR, glm::vec4(0, 1, 0, 1));

        // Blue material
        m_BlueMaterial = std::make_unique<Graphics::Material>();
        m_BlueMaterial->SetShader(shader);
        m_BlueMaterial->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_TestTexture.get());
        m_BlueMaterial->SetVector(Graphics::Material::ALBEDO_COLOR, glm::vec4(0, 0, 1, 1));

        YAMEN_CORE_INFO("Test materials created");
    }

    void DemoScene::Update(float deltaTime) {
        // Rotate meshes
        m_Rotation += deltaTime * 0.5f;

        // TODO: Add camera controls
    }

    void DemoScene::Render() {
        // === 3D Rendering ===
        if (m_Show3D) {
            m_Renderer3D->BeginScene(m_Camera3D.get());
            m_Renderer3D->SetWireframe(m_ShowWireframe);

            // Update sun light from ImGui controls
            m_SunLight.direction = glm::normalize(m_LightDirection);
            m_SunLight.color = m_LightColor;
            
            m_Renderer3D->SubmitLight(m_SunLight);
            m_Renderer3D->SubmitLight(m_PointLight1);
            m_Renderer3D->SubmitLight(m_PointLight2);

            // Transforms
            glm::mat4 planeTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, 0.0f));
            glm::mat4 cubeTransform = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 0.0f, 0.0f));
            cubeTransform = glm::rotate(cubeTransform, m_Rotation, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 sphereTransform = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 0.0f));
            sphereTransform = glm::rotate(sphereTransform, m_Rotation * 0.7f, glm::vec3(1.0f, 0.0f, 1.0f));

            if (m_UseMaterials) {
                // Render with materials (different colors)
                m_Renderer3D->DrawMesh(m_PlaneMesh.get(), planeTransform, m_GreenMaterial.get());
                m_Renderer3D->DrawMesh(m_CubeMesh.get(), cubeTransform, m_RedMaterial.get());
                m_Renderer3D->DrawMesh(m_SphereMesh.get(), sphereTransform, m_BlueMaterial.get());
            } else {
                // Original rendering
                m_Renderer3D->DrawMesh(m_PlaneMesh.get(), planeTransform, nullptr, glm::vec4(0.3f, 0.5f, 0.3f, 1.0f));
                m_Renderer3D->DrawMesh(m_CubeMesh.get(), cubeTransform, nullptr, glm::vec4(1.0f, 0.3f, 0.3f, 1.0f));
                m_Renderer3D->DrawMesh(m_SphereMesh.get(), sphereTransform, nullptr, glm::vec4(0.3f, 0.3f, 1.0f, 1.0f));
            }

            m_Renderer3D->EndScene();
        }

        // === 2D Rendering ===
        if (m_Show2D) {
            m_Renderer2D->BeginScene(m_Camera2D.get());

            // Draw some test quads
            m_Renderer2D->DrawQuad(
                glm::vec2(100.0f, 100.0f),
                glm::vec2(100.0f, 100.0f),
                glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)
            );

            m_Renderer2D->DrawQuad(
                glm::vec2(220.0f, 100.0f),
                glm::vec2(100.0f, 100.0f),
                glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)
            );

            m_Renderer2D->DrawQuad(
                glm::vec2(340.0f, 100.0f),
                glm::vec2(100.0f, 100.0f),
                glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)
            );

            m_Renderer2D->EndScene();
        }
    }
    
    void DemoScene::RenderImGui() {
        ImGui::Begin("Demo Scene Controls");

        ImGui::Text("Phase 3 Graphics Demo");
        ImGui::Separator();

        ImGui::Checkbox("Show 2D", &m_Show2D);
        ImGui::Checkbox("Show 3D", &m_Show3D);
        ImGui::Checkbox("Wireframe", &m_ShowWireframe);
        ImGui::Checkbox("Use Materials", &m_UseMaterials);

        ImGui::Separator();
        ImGui::Text("Lighting");
        ImGui::Text("Lights: %zu active", m_LightManager->GetLightCount());
        ImGui::SliderFloat3("Light Direction", &m_LightDirection.x, -1.0f, 1.0f);
        ImGui::ColorEdit3("Light Color", &m_LightColor.x);
        ImGui::End();
    }

} // namespace Yamen