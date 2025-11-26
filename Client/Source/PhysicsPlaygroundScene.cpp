#include "Client/PhysicsPlaygroundScene.h"
#include "Client/CameraController.h"
#include "ECS/Components.h"
#include "ECS/Systems/RenderSystem.h"
#include "ECS/Systems/CameraSystem.h"
#include "ECS/Systems/PhysicsSystem.h"
#include "ECS/Systems/ScriptSystem.h"
#include "Graphics/Mesh/MeshBuilder.h"
#include <Core/Logging/Logger.h>
#include <imgui.h>
#include <random>

namespace Yamen {

    PhysicsPlaygroundScene::PhysicsPlaygroundScene(Graphics::GraphicsDevice& device)
        : m_Device(device)
    {
    }

    PhysicsPlaygroundScene::~PhysicsPlaygroundScene() {
    }

    bool PhysicsPlaygroundScene::Initialize() {
        m_Scene = std::make_unique<ECS::Scene>("Physics Playground");
        
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

        Graphics::MeshBuilder::CreateSphere(vertices, indices, 0.5f, 16, 16);
        m_SphereMesh = std::make_shared<Graphics::Mesh>(m_Device);
        m_SphereMesh->Create(vertices, indices);

        m_Shader = std::make_unique<Graphics::Shader>(m_Device);
        m_Shader->CreateFromFiles(
            "C:/dev/C3Renderer/Yamen/Assets/Shaders/Basic3D.hlsl",
            "C:/dev/C3Renderer/Yamen/Assets/Shaders/Basic3D.hlsl",
            "VSMain", "PSMain"
        );

        m_WhiteTexture = std::make_shared<Graphics::Texture2D>(m_Device);
        uint32_t whitePixel = 0xFFFFFFFF;
        m_WhiteTexture->Create(1, 1, Graphics::TextureFormat::R8G8B8A8_UNORM, &whitePixel);

        // Create camera
        auto cameraEntity = m_Scene->CreateEntity("MainCamera");
        auto& cameraComp = cameraEntity.AddComponent<ECS::CameraComponent>();
        cameraComp.Primary = true;
        cameraComp.Camera.SetFOV(60.0f);
        cameraComp.Camera.SetAspectRatio(16.0f/9.0f);
        cameraComp.Camera.SetClipPlanes(0.1f, 1000.0f);
        
        auto& cameraTransform = cameraEntity.GetComponent<ECS::TransformComponent>();
        cameraTransform.Translation = glm::vec3(0.0f, 5.0f, -15.0f);
        
        auto& script = cameraEntity.AddComponent<ECS::NativeScriptComponent>();
        script.Bind<Client::CameraController>();

        // Create light
        auto lightEntity = m_Scene->CreateEntity("Sun");
        auto& lightComp = lightEntity.AddComponent<ECS::LightComponent>();
        lightComp.LightData = Graphics::Light::CreateDirectional(
            glm::normalize(glm::vec3(-0.5f, -1.0f, -0.3f)),
            glm::vec3(1.0f, 1.0f, 1.0f),
            2.0f
        );
        lightComp.Active = true;

        // Create physics playground
        CreateGround();
        CreatePyramid();
        CreateDominoChain();
        CreateBouncyBalls();
        CreateHeavyBox();

        YAMEN_CORE_INFO("Physics Playground Scene Initialized");
        return true;
    }

    void PhysicsPlaygroundScene::CreateGround() {
        auto ground = m_Scene->CreateEntity("Ground");
        auto& mesh = ground.AddComponent<ECS::MeshComponent>();
        mesh.Mesh = m_CubeMesh;
        
        auto material = std::make_shared<Graphics::Material>();
        material->SetShader(m_Shader.get());
        material->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_WhiteTexture.get());
        material->SetVector(Graphics::Material::ALBEDO_COLOR, glm::vec4(0.3f, 0.3f, 0.3f, 1.0f));
        mesh.Material = material;
        
        auto& transform = ground.GetComponent<ECS::TransformComponent>();
        transform.Translation = glm::vec3(0.0f, -1.0f, 0.0f);
        transform.Scale = glm::vec3(30.0f, 1.0f, 30.0f);
        
        auto& rb = ground.AddComponent<ECS::RigidBodyComponent>();
        rb.Type = ECS::BodyType::Static;
        
        ECS::BoxCollider collider;
        collider.HalfExtents = glm::vec3(15.0f, 0.5f, 15.0f);
        ground.AddComponent<ECS::ColliderComponent>(collider);
    }

    void PhysicsPlaygroundScene::CreatePyramid() {
        // Create a pyramid of boxes
        int levels = 5;
        float boxSize = 1.0f;
        float startX = -5.0f;
        float startY = 0.5f;
        float startZ = 0.0f;

        for (int level = 0; level < levels; ++level) {
            int boxesInLevel = levels - level;
            for (int i = 0; i < boxesInLevel; ++i) {
                auto box = m_Scene->CreateEntity("PyramidBox");
                auto& mesh = box.AddComponent<ECS::MeshComponent>();
                mesh.Mesh = m_CubeMesh;
                
                auto material = std::make_shared<Graphics::Material>();
                material->SetShader(m_Shader.get());
                material->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_WhiteTexture.get());
                
                // Color based on level
                float hue = (float)level / levels;
                glm::vec3 color = glm::vec3(1.0f - hue, hue, 0.5f);
                material->SetVector(Graphics::Material::ALBEDO_COLOR, glm::vec4(color, 1.0f));
                mesh.Material = material;
                
                auto& transform = box.GetComponent<ECS::TransformComponent>();
                transform.Translation = glm::vec3(
                    startX + i * boxSize + level * boxSize * 0.5f,
                    startY + level * boxSize,
                    startZ
                );
                transform.Scale = glm::vec3(0.9f);
                
                auto& rb = box.AddComponent<ECS::RigidBodyComponent>();
                rb.Type = ECS::BodyType::Dynamic;
                rb.Mass = 1.0f;
                
                ECS::BoxCollider collider;
                collider.HalfExtents = glm::vec3(0.45f);
                box.AddComponent<ECS::ColliderComponent>(collider);
            }
        }
    }

    void PhysicsPlaygroundScene::CreateDominoChain() {
        // Create a chain of domino pieces
        int dominoCount = 10;
        float spacing = 1.5f;
        float startX = 5.0f;
        float startZ = -5.0f;

        for (int i = 0; i < dominoCount; ++i) {
            auto domino = m_Scene->CreateEntity("Domino");
            auto& mesh = domino.AddComponent<ECS::MeshComponent>();
            mesh.Mesh = m_CubeMesh;
            
            auto material = std::make_shared<Graphics::Material>();
            material->SetShader(m_Shader.get());
            material->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_WhiteTexture.get());
            material->SetVector(Graphics::Material::ALBEDO_COLOR, glm::vec4(0.8f, 0.2f, 0.2f, 1.0f));
            mesh.Material = material;
            
            auto& transform = domino.GetComponent<ECS::TransformComponent>();
            transform.Translation = glm::vec3(startX, 1.5f, startZ + i * spacing);
            transform.Scale = glm::vec3(0.2f, 3.0f, 1.0f);
            
            auto& rb = domino.AddComponent<ECS::RigidBodyComponent>();
            rb.Type = ECS::BodyType::Dynamic;
            rb.Mass = 0.5f;
            
            ECS::BoxCollider collider;
            collider.HalfExtents = glm::vec3(0.1f, 1.5f, 0.5f);
            domino.AddComponent<ECS::ColliderComponent>(collider);
        }
    }

    void PhysicsPlaygroundScene::CreateBouncyBalls() {
        // Create bouncy spheres
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> posX(-3.0f, 3.0f);
        std::uniform_real_distribution<float> posZ(-3.0f, 3.0f);
        std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);

        for (int i = 0; i < 5; ++i) {
            auto ball = m_Scene->CreateEntity("BouncyBall");
            auto& mesh = ball.AddComponent<ECS::MeshComponent>();
            mesh.Mesh = m_SphereMesh;
            
            auto material = std::make_shared<Graphics::Material>();
            material->SetShader(m_Shader.get());
            material->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_WhiteTexture.get());
            material->SetVector(Graphics::Material::ALBEDO_COLOR, 
                glm::vec4(colorDist(gen), colorDist(gen), colorDist(gen), 1.0f));
            mesh.Material = material;
            
            auto& transform = ball.GetComponent<ECS::TransformComponent>();
            transform.Translation = glm::vec3(posX(gen), 5.0f + i * 2.0f, posZ(gen));
            
            auto& rb = ball.AddComponent<ECS::RigidBodyComponent>();
            rb.Type = ECS::BodyType::Dynamic;
            rb.Mass = 0.5f;
            
            ECS::SphereCollider collider;
            collider.Radius = 0.5f;
            auto& col = ball.AddComponent<ECS::ColliderComponent>(collider);
            col.Bounciness = 0.9f; // Very bouncy!
        }
    }

    void PhysicsPlaygroundScene::CreateHeavyBox() {
        // Create a heavy wrecking ball
        auto ball = m_Scene->CreateEntity("WreckingBall");
        auto& mesh = ball.AddComponent<ECS::MeshComponent>();
        mesh.Mesh = m_SphereMesh;
        
        auto material = std::make_shared<Graphics::Material>();
        material->SetShader(m_Shader.get());
        material->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_WhiteTexture.get());
        material->SetVector(Graphics::Material::ALBEDO_COLOR, glm::vec4(0.2f, 0.2f, 0.2f, 1.0f));
        mesh.Material = material;
        
        auto& transform = ball.GetComponent<ECS::TransformComponent>();
        transform.Translation = glm::vec3(-8.0f, 8.0f, 0.0f);
        transform.Scale = glm::vec3(2.0f);
        
        auto& rb = ball.AddComponent<ECS::RigidBodyComponent>();
        rb.Type = ECS::BodyType::Dynamic;
        rb.Mass = 10.0f; // Heavy!
        rb.Velocity = glm::vec3(5.0f, 0.0f, 0.0f); // Initial push
        
        ECS::SphereCollider collider;
        collider.Radius = 1.0f;
        ball.AddComponent<ECS::ColliderComponent>(collider);
    }

    void PhysicsPlaygroundScene::SpawnRandomObject() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> posX(-5.0f, 5.0f);
        std::uniform_real_distribution<float> posZ(-5.0f, 5.0f);
        std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);
        std::uniform_int_distribution<int> typeDist(0, 1);

        bool isSphere = typeDist(gen) == 0;
        auto entity = m_Scene->CreateEntity(isSphere ? "RandomSphere" : "RandomBox");
        
        auto& mesh = entity.AddComponent<ECS::MeshComponent>();
        mesh.Mesh = isSphere ? m_SphereMesh : m_CubeMesh;
        
        auto material = std::make_shared<Graphics::Material>();
        material->SetShader(m_Shader.get());
        material->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_WhiteTexture.get());
        material->SetVector(Graphics::Material::ALBEDO_COLOR, 
            glm::vec4(colorDist(gen), colorDist(gen), colorDist(gen), 1.0f));
        mesh.Material = material;
        
        auto& transform = entity.GetComponent<ECS::TransformComponent>();
        transform.Translation = glm::vec3(posX(gen), 10.0f, posZ(gen));
        
        auto& rb = entity.AddComponent<ECS::RigidBodyComponent>();
        rb.Type = ECS::BodyType::Dynamic;
        rb.Mass = 1.0f;
        
        if (isSphere) {
            ECS::SphereCollider collider;
            collider.Radius = 0.5f;
            entity.AddComponent<ECS::ColliderComponent>(collider);
        } else {
            ECS::BoxCollider collider;
            collider.HalfExtents = glm::vec3(0.5f);
            entity.AddComponent<ECS::ColliderComponent>(collider);
        }
    }

    void PhysicsPlaygroundScene::Update(float deltaTime) {
        if (m_Scene) {
            m_Scene->OnUpdate(deltaTime);
        }

        // Auto-spawn objects
        if (m_AutoSpawn) {
            m_SpawnTimer += deltaTime;
            if (m_SpawnTimer >= 2.0f) {
                SpawnRandomObject();
                m_SpawnTimer = 0.0f;
            }
        }
    }

    void PhysicsPlaygroundScene::Render() {
        if (m_Scene) {
            m_Scene->OnRender();
        }
    }

    void PhysicsPlaygroundScene::RenderImGui() {
        if (!m_Scene) return;

        ImGui::Begin("Physics Playground Controls");
        
        ImGui::Text("Interactive Physics Demo");
        ImGui::Separator();
        
        if (ImGui::Button("Spawn Random Object")) {
            SpawnRandomObject();
        }
        
        ImGui::Checkbox("Auto-Spawn Objects", &m_AutoSpawn);
        
        ImGui::Separator();
        ImGui::Text("Physics Settings");
        
        auto* physicsSystem = m_Scene->GetSystem<ECS::PhysicsSystem>();
        if (physicsSystem) {
            ImGui::DragFloat3("Gravity", &physicsSystem->Gravity.x, 0.1f, -50.0f, 50.0f);
            ImGui::DragInt("SubSteps", &physicsSystem->SubSteps, 1, 1, 10);
        }
        
        ImGui::Separator();
        ImGui::Text("Entity Count: %d", m_Scene->Registry().view<ECS::TagComponent>().size());
        
        ImGui::End();
    }

} // namespace Yamen
