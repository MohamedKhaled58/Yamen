#include "Client/MultiCameraScene.h"
#include "Client/CameraController.h"
#include "ECS/Components.h"
#include "ECS/Systems/RenderSystem.h"
#include "ECS/Systems/CameraSystem.h"
#include "ECS/Systems/PhysicsSystem.h"
#include "ECS/Systems/ScriptSystem.h"
#include "Graphics/Mesh/MeshBuilder.h"
#include <Core/Logging/Logger.h>
#include <imgui.h>

namespace Yamen {

    MultiCameraScene::MultiCameraScene(Graphics::GraphicsDevice& device)
        : m_Device(device)
    {
    }

    MultiCameraScene::~MultiCameraScene() {
    }

    bool MultiCameraScene::Initialize() {
        m_Scene = std::make_unique<ECS::Scene>("Multi-Camera Demo");
        
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
        m_Scene->AddSystem<ECS::RenderSystem>(m_Device, m_Renderer3D.get(), m_Renderer2D.get());
        m_Scene->OnInit();

        // Create meshes
        std::vector<Graphics::Vertex3D> vertices;
        std::vector<uint32_t> indices;
        
        Graphics::MeshBuilder::CreateCube(vertices, indices, 1.0f);
        m_CubeMesh = std::make_shared<Graphics::Mesh>(m_Device);
        m_CubeMesh->Create(vertices, indices);

        m_Shader = std::make_unique<Graphics::Shader>(m_Device);
        m_Shader->CreateFromFiles(
            "C:/dev/C3Renderer/Yamen/Assets/Shaders/Basic3D.hlsl",
            "C:/dev/C3Renderer/Yamen/Assets/Shaders/Basic3D.hlsl",
            "VSMain", "PSMain"
        );

        m_WhiteTexture = std::make_shared<Graphics::Texture2D>(m_Device);
        uint32_t whitePixel = 0xFFFFFFFF;
        m_WhiteTexture->Create(1, 1, Graphics::TextureFormat::R8G8B8A8_UNORM, &whitePixel);

        // Create cameras
        CreateMainCamera();
        CreateTopDownCamera();
        CreateFollowCamera();

        // Create environment
        // Ground
        auto ground = m_Scene->CreateEntity("Ground");
        auto& mesh = ground.AddComponent<ECS::MeshComponent>();
        mesh.Mesh = m_CubeMesh;
        
        auto material = std::make_shared<Graphics::Material>();
        material->SetShader(m_Shader.get());
        material->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_WhiteTexture.get());
        material->SetVector(Graphics::Material::ALBEDO_COLOR, glm::vec4(0.3f, 0.5f, 0.3f, 1.0f));
        mesh.Material = material;
        
        auto& transform = ground.GetComponent<ECS::TransformComponent>();
        transform.Translation = glm::vec3(0.0f, -1.0f, 0.0f);
        transform.Scale = glm::vec3(30.0f, 1.0f, 30.0f);

        // Create player cube
        auto player = m_Scene->CreateEntity("Player");
        auto& playerMesh = player.AddComponent<ECS::MeshComponent>();
        playerMesh.Mesh = m_CubeMesh;
        
        auto playerMat = std::make_shared<Graphics::Material>();
        playerMat->SetShader(m_Shader.get());
        playerMat->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_WhiteTexture.get());
        playerMat->SetVector(Graphics::Material::ALBEDO_COLOR, glm::vec4(1.0f, 0.3f, 0.3f, 1.0f));
        playerMesh.Material = playerMat;
        
        auto& playerTransform = player.GetComponent<ECS::TransformComponent>();
        playerTransform.Translation = glm::vec3(0.0f, 1.0f, 0.0f);
        playerTransform.Scale = glm::vec3(1.5f);
        
        m_PlayerCube = player;
        return true;
    }

    void MultiCameraScene::CreateMainCamera() {
        auto camera = m_Scene->CreateEntity("MainCamera");
        auto& cameraComp = camera.AddComponent<ECS::CameraComponent>();
        cameraComp.Primary = true;
        cameraComp.Camera.SetFOV(60.0f);
        cameraComp.Camera.SetAspectRatio(16.0f/9.0f);
        cameraComp.Camera.SetClipPlanes(0.1f, 1000.0f);
        
        auto& transform = camera.GetComponent<ECS::TransformComponent>();
        transform.Translation = glm::vec3(0.0f, 5.0f, -15.0f);
        transform.Rotation = glm::quat(glm::vec3(0.0f, glm::radians(90.0f), 0.0f));
        
        auto& script = camera.AddComponent<ECS::NativeScriptComponent>();
        script.Bind<Client::CameraController>();
        
        m_MainCamera = camera;
    }

    void MultiCameraScene::CreateTopDownCamera() {
        auto camera = m_Scene->CreateEntity("TopDownCamera");
        auto& cameraComp = camera.AddComponent<ECS::CameraComponent>();
        cameraComp.Primary = false;
        cameraComp.Camera.SetFOV(60.0f);
        cameraComp.Camera.SetAspectRatio(16.0f/9.0f);
        cameraComp.Camera.SetClipPlanes(0.1f, 1000.0f);
        
        auto& transform = camera.GetComponent<ECS::TransformComponent>();
        transform.Translation = glm::vec3(0.0f, 30.0f, 0.0f);
        transform.Rotation = glm::angleAxis(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        
        m_TopDownCamera = camera;
    }

    void MultiCameraScene::CreateFollowCamera() {
        auto camera = m_Scene->CreateEntity("FollowCamera");
        auto& cameraComp = camera.AddComponent<ECS::CameraComponent>();
        cameraComp.Primary = false;
        cameraComp.Camera.SetFOV(60.0f);
        cameraComp.Camera.SetAspectRatio(16.0f/9.0f);
        cameraComp.Camera.SetClipPlanes(0.1f, 1000.0f);
        
        m_FollowCamera = camera;

        // Create obstacles
        for (int i = 0; i < 5; ++i) {
            auto obstacle = m_Scene->CreateEntity("Obstacle");
            auto& obsMesh = obstacle.AddComponent<ECS::MeshComponent>();
            obsMesh.Mesh = m_CubeMesh;
            
            auto obsMat = std::make_shared<Graphics::Material>();
            obsMat->SetShader(m_Shader.get());
            obsMat->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_WhiteTexture.get());
            obsMat->SetVector(Graphics::Material::ALBEDO_COLOR, 
                glm::vec4(0.5f + i * 0.1f, 0.5f, 1.0f - i * 0.1f, 1.0f));
            obsMesh.Material = obsMat;
            
            auto& obsTransform = obstacle.GetComponent<ECS::TransformComponent>();
            obsTransform.Translation = glm::vec3(
                -10.0f + i * 5.0f,
                2.0f,
                5.0f
            );
            obsTransform.Scale = glm::vec3(1.0f, 4.0f, 1.0f);
        }

        // Create light
        auto light = m_Scene->CreateEntity("Sun");
       auto& lightComp = light.AddComponent<ECS::LightComponent>();
lightComp.LightData = Graphics::Light::CreateDirectional(
    glm::normalize(glm::vec3(-0.5f, -1.0f, -0.3f)),
    glm::vec3(1.0f, 1.0f, 1.0f),
    2.0f
);
        lightComp.Active = true;

        YAMEN_CORE_INFO("Multi-Camera Scene Initialized");
    }

    void MultiCameraScene::Update(float deltaTime) {
        if (m_Scene) {
            if (m_FollowCamera != entt::null && m_PlayerCube != entt::null) {
                auto& followTransform = m_Scene->Registry().get<ECS::TransformComponent>(m_FollowCamera);
                auto& playerTransform = m_Scene->Registry().get<ECS::TransformComponent>(m_PlayerCube);
                
                glm::vec3 offset = glm::vec3(0.0f, 3.0f, -8.0f);
                followTransform.Translation = playerTransform.Translation + offset;
                
                glm::vec3 direction = glm::normalize(playerTransform.Translation - followTransform.Translation);
                float pitch = asin(-direction.y);
                float yaw = atan2(direction.x, direction.z);
                followTransform.Rotation = glm::angleAxis(yaw, glm::vec3(0.0f, 1.0f, 0.0f)) * 
                                          glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f));
            }
            
            m_Scene->OnUpdate(deltaTime);
        }
    }

    void MultiCameraScene::Render() {
        if (m_Scene) {
            m_Scene->OnRender();
        }
    }

    void MultiCameraScene::RenderImGui() {
        if (!m_Scene) return;

        ImGui::Begin("Multi-Camera Controls");
        
        ImGui::Text("Camera View Modes");
        ImGui::Separator();
        
        if (ImGui::RadioButton("Single View", m_ViewMode == ViewMode::Single)) {
            m_ViewMode = ViewMode::Single;
        }
        if (ImGui::RadioButton("Split Screen", m_ViewMode == ViewMode::SplitScreen)) {
            m_ViewMode = ViewMode::SplitScreen;
        }
        if (ImGui::RadioButton("Picture-in-Picture", m_ViewMode == ViewMode::PictureInPicture)) {
            m_ViewMode = ViewMode::PictureInPicture;
        }
        
        ImGui::Separator();
        ImGui::Text("Active Cameras:");
        ImGui::BulletText("Main Camera (FPS)");
        ImGui::BulletText("Top-Down Camera (Minimap)");
        ImGui::BulletText("Follow Camera (Third-Person)");
        
        ImGui::Separator();
        ImGui::Text("Note: Full multi-viewport rendering");
        ImGui::Text("requires render target support.");
        ImGui::Text("This demo shows the camera setup.");
        
        ImGui::End();
    }

} // namespace Yamen