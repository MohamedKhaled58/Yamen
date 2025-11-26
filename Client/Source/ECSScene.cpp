#include "Client/ECSScene.h"
#include "Client/CameraController.h"
#include "ECS/Components.h"
#include "ECS/Systems/RenderSystem.h"
#include "ECS/Systems/CameraSystem.h"
#include "ECS/Systems/CameraSystem.h"
#include "ECS/Systems/PhysicsSystem.h"
#include "ECS/Systems/ScriptSystem.h"
#include "Graphics/Mesh/MeshBuilder.h"
#include "Graphics/Texture/TextureLoader.h"
#include <Core/Logging/Logger.h>
#include <imgui.h>

namespace Yamen {

    ECSScene::ECSScene(Graphics::GraphicsDevice& device)
        : m_Device(device)
    {
    }

    ECSScene::~ECSScene() {
    }

    bool ECSScene::Initialize() {
        // Create scene
        m_Scene = std::make_unique<ECS::Scene>("Main Scene");
        
        // Create renderers (these should ideally be passed from GameLayer)
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

        // Add systems (they will be sorted by priority automatically)
        m_Scene->AddSystem<ECS::CameraSystem>();
        m_Scene->AddSystem<ECS::ScriptSystem>();
        m_Scene->AddSystem<ECS::PhysicsSystem>();
        m_Scene->AddSystem<ECS::RenderSystem>(m_Device, m_Renderer3D.get(), m_Renderer2D.get());

        // Initialize systems
        m_Scene->OnInit();

        // Create resources
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

        // Create white texture for materials (shader expects a texture)
        m_WhiteTexture = std::make_shared<Graphics::Texture2D>(m_Device);
        uint32_t whitePixel = 0xFFFFFFFF;
        m_WhiteTexture->Create(1, 1, Graphics::TextureFormat::R8G8B8A8_UNORM, &whitePixel);
        
        m_Material = std::make_shared<Graphics::Material>();
        m_Material->SetShader(m_Shader.get());
        m_Material->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_WhiteTexture.get());
        m_Material->SetVector(Graphics::Material::ALBEDO_COLOR, glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)); // Bright orange

        // Create Camera Entity with controller script
        auto cameraEntity = m_Scene->CreateEntity("MainCamera");
        auto& cameraComp = cameraEntity.AddComponent<ECS::CameraComponent>();
        cameraComp.Primary = true;
        cameraComp.Camera.SetFOV(60.0f);
        cameraComp.Camera.SetAspectRatio(16.0f/9.0f);
        cameraComp.Camera.SetClipPlanes(0.1f, 1000.0f);
        
        auto& cameraTransform = cameraEntity.GetComponent<ECS::TransformComponent>();
        cameraTransform.Translation = glm::vec3(0.0f, 2.0f, 0.0f);
        // Rotation is handled by CameraController now

        // Add camera controller script
        auto& script = cameraEntity.AddComponent<ECS::NativeScriptComponent>();
        script.Bind<Client::CameraController>();

        // Create multiple cubes to see which direction camera is looking
        // Cube in front (+Z)
        auto cubeZ = m_Scene->CreateEntity("Cube_Z");
        auto& meshZ = cubeZ.AddComponent<ECS::MeshComponent>();
        meshZ.Mesh = m_CubeMesh;
        meshZ.Material = m_Material;
        cubeZ.GetComponent<ECS::TransformComponent>().Translation = glm::vec3(0.0f, 2.0f, 5.0f);
        
        // Cube behind (-Z) - RED
        auto cubeNZ = m_Scene->CreateEntity("Cube_NZ");
        auto& meshNZ = cubeNZ.AddComponent<ECS::MeshComponent>();
        meshNZ.Mesh = m_CubeMesh;
        auto matRed = std::make_shared<Graphics::Material>();
        matRed->SetShader(m_Shader.get());
        matRed->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_WhiteTexture.get());
        matRed->SetVector(Graphics::Material::ALBEDO_COLOR, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        meshNZ.Material = matRed;
        cubeNZ.GetComponent<ECS::TransformComponent>().Translation = glm::vec3(0.0f, 2.0f, -5.0f);
        
        // Cube to right (+X) - GREEN
        auto cubeX = m_Scene->CreateEntity("Cube_X");
        auto& meshX = cubeX.AddComponent<ECS::MeshComponent>();
        meshX.Mesh = m_CubeMesh;
        auto matGreen = std::make_shared<Graphics::Material>();
        matGreen->SetShader(m_Shader.get());
        matGreen->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_WhiteTexture.get());
        matGreen->SetVector(Graphics::Material::ALBEDO_COLOR, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
        meshX.Material = matGreen;
        cubeX.GetComponent<ECS::TransformComponent>().Translation = glm::vec3(5.0f, 2.0f, 0.0f);
        
        // Cube to left (-X) - BLUE
        auto cubeNX = m_Scene->CreateEntity("Cube_NX");
        auto& meshNX = cubeNX.AddComponent<ECS::MeshComponent>();
        meshNX.Mesh = m_CubeMesh;
        auto matBlue = std::make_shared<Graphics::Material>();
        matBlue->SetShader(m_Shader.get());
        matBlue->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_WhiteTexture.get());
        matBlue->SetVector(Graphics::Material::ALBEDO_COLOR, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
        meshNX.Material = matBlue;
        cubeNX.GetComponent<ECS::TransformComponent>().Translation = glm::vec3(-5.0f, 2.0f, 0.0f);

        // --- COORDINATE GIZMOS ---
        // X-Axis (Red)
        auto axisX = m_Scene->CreateEntity("Axis_X");
        auto& meshAxisX = axisX.AddComponent<ECS::MeshComponent>();
        meshAxisX.Mesh = m_CubeMesh;
        auto matAxisX = std::make_shared<Graphics::Material>();
        matAxisX->SetShader(m_Shader.get());
        matAxisX->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_WhiteTexture.get());
        matAxisX->SetVector(Graphics::Material::ALBEDO_COLOR, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // Red
        meshAxisX.Material = matAxisX;
        auto& transX = axisX.GetComponent<ECS::TransformComponent>();
        transX.Translation = glm::vec3(2.5f, 0.0f, 0.0f);
        transX.Scale = glm::vec3(5.0f, 0.05f, 0.05f);

        // Y-Axis (Green)
        auto axisY = m_Scene->CreateEntity("Axis_Y");
        auto& meshAxisY = axisY.AddComponent<ECS::MeshComponent>();
        meshAxisY.Mesh = m_CubeMesh;
        auto matAxisY = std::make_shared<Graphics::Material>();
        matAxisY->SetShader(m_Shader.get());
        matAxisY->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_WhiteTexture.get());
        matAxisY->SetVector(Graphics::Material::ALBEDO_COLOR, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)); // Green
        meshAxisY.Material = matAxisY;
        auto& transY = axisY.GetComponent<ECS::TransformComponent>();
        transY.Translation = glm::vec3(0.0f, 2.5f, 0.0f);
        transY.Scale = glm::vec3(0.05f, 5.0f, 0.05f);

        // Z-Axis (Blue)
        auto axisZ = m_Scene->CreateEntity("Axis_Z");
        auto& meshAxisZ = axisZ.AddComponent<ECS::MeshComponent>();
        meshAxisZ.Mesh = m_CubeMesh;
        auto matAxisZ = std::make_shared<Graphics::Material>();
        matAxisZ->SetShader(m_Shader.get());
        matAxisZ->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_WhiteTexture.get());
        matAxisZ->SetVector(Graphics::Material::ALBEDO_COLOR, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)); // Blue
        meshAxisZ.Material = matAxisZ;
        auto& transZ = axisZ.GetComponent<ECS::TransformComponent>();
        transZ.Translation = glm::vec3(0.0f, 0.0f, 2.5f);
        transZ.Scale = glm::vec3(0.05f, 0.05f, 5.0f);

        // --- PHYSICS DEMO ---
        // 1. Ground Plane (Static Box)
        auto ground = m_Scene->CreateEntity("Ground");
        auto& groundMesh = ground.AddComponent<ECS::MeshComponent>();
        groundMesh.Mesh = m_CubeMesh;
        auto groundMat = std::make_shared<Graphics::Material>();
        groundMat->SetShader(m_Shader.get());
        groundMat->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_WhiteTexture.get());
        groundMat->SetVector(Graphics::Material::ALBEDO_COLOR, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)); // Grey
        groundMesh.Material = groundMat;
        
        auto& groundTrans = ground.GetComponent<ECS::TransformComponent>();
        groundTrans.Translation = glm::vec3(0.0f, -2.0f, 0.0f);
        groundTrans.Scale = glm::vec3(20.0f, 1.0f, 20.0f);
        
        auto& groundRb = ground.AddComponent<ECS::RigidBodyComponent>();
        groundRb.Type = ECS::BodyType::Static;
        
        ECS::BoxCollider groundBox;
        groundBox.HalfExtents = glm::vec3(10.0f, 0.5f, 10.0f);
        ground.AddComponent<ECS::ColliderComponent>(groundBox);

        // 2. Falling Box (Dynamic)
        auto physBox = m_Scene->CreateEntity("PhysicsBox");
        auto& physBoxMesh = physBox.AddComponent<ECS::MeshComponent>();
        physBoxMesh.Mesh = m_CubeMesh;
        auto physBoxMat = std::make_shared<Graphics::Material>();
        physBoxMat->SetShader(m_Shader.get());
        physBoxMat->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_WhiteTexture.get());
        physBoxMat->SetVector(Graphics::Material::ALBEDO_COLOR, glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)); // Orange
        physBoxMesh.Material = physBoxMat;
        
        auto& physBoxTrans = physBox.GetComponent<ECS::TransformComponent>();
        physBoxTrans.Translation = glm::vec3(-2.0f, 10.0f, 0.0f); // Start high up
        
        auto& physBoxRb = physBox.AddComponent<ECS::RigidBodyComponent>();
        physBoxRb.Mass = 1.0f;
        physBoxRb.Type = ECS::BodyType::Dynamic;
        
        ECS::BoxCollider physBoxCol;
        physBoxCol.HalfExtents = glm::vec3(0.5f, 0.5f, 0.5f);
        physBox.AddComponent<ECS::ColliderComponent>(physBoxCol);

        // 3. Bouncing Sphere (Dynamic)
        auto physSphere = m_Scene->CreateEntity("PhysicsSphere");
        auto& physSphereMesh = physSphere.AddComponent<ECS::MeshComponent>();
        // Using cube mesh for now as we don't have a sphere mesh generator yet, but physics will be sphere
        physSphereMesh.Mesh = m_CubeMesh; 
        auto physSphereMat = std::make_shared<Graphics::Material>();
        physSphereMat->SetShader(m_Shader.get());
        physSphereMat->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_WhiteTexture.get());
        physSphereMat->SetVector(Graphics::Material::ALBEDO_COLOR, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)); // Cyan
        physSphereMesh.Material = physSphereMat;
        
        auto& physSphereTrans = physSphere.GetComponent<ECS::TransformComponent>();
        physSphereTrans.Translation = glm::vec3(2.0f, 12.0f, 0.0f);
        
        auto& physSphereRb = physSphere.AddComponent<ECS::RigidBodyComponent>();
        physSphereRb.Mass = 1.0f;
        physSphereRb.Type = ECS::BodyType::Dynamic;
        
        ECS::SphereCollider physSphereCol;
        physSphereCol.Radius = 0.5f;
        auto& sphereColComp = physSphere.AddComponent<ECS::ColliderComponent>(physSphereCol);
        sphereColComp.Bounciness = 0.8f; // High bounciness

        // Create Light Entity (brighter directional light from above)
        auto lightEntity = m_Scene->CreateEntity("Sun");
        auto& lightComp = lightEntity.AddComponent<ECS::LightComponent>();
        lightComp.LightData = Graphics::Light::CreateDirectional(
            glm::normalize(glm::vec3(-0.3f, -1.0f, -0.3f)), // Direction: from top-front
            glm::vec3(1.0f, 1.0f, 1.0f), // White light
            2.0f // Brighter intensity
        );
        lightComp.Active = true;
        lightComp.CastShadows = false; // Disable shadows for now

        auto entityCount = m_Scene->Registry().view<ECS::TagComponent>().size();
        YAMEN_CORE_INFO("ECS Scene Initialized with {} entities", entityCount);
        return true;
    }

    void ECSScene::Update(float deltaTime) {
        if (m_Scene) {
            m_Scene->OnUpdate(deltaTime);
        }
    }

    void ECSScene::Render() {
        if (m_Scene) {
            m_Scene->OnRender();
        }
    }

    void ECSScene::RenderImGui() {
        if (!m_Scene) return;

        // --- Scene Hierarchy ---
        ImGui::Begin("Scene Hierarchy");
        
        auto view = m_Scene->Registry().view<ECS::TagComponent>();
        for (auto entity : view) {
            auto& tag = view.get<ECS::TagComponent>(entity);
            
            ImGuiTreeNodeFlags flags = ((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
            bool opened = ImGui::TreeNodeEx((void*)(uint64_t)entity, flags, "%s", tag.Tag.c_str());
            
            if (ImGui::IsItemClicked()) {
                m_SelectedEntity = entity;
            }

            if (opened) {
                ImGui::TreePop();
            }
        }
        
        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
            m_SelectedEntity = entt::null;
        }

        // Right-click context menu
        if (ImGui::BeginPopupContextWindow()) {
            if (ImGui::MenuItem("Create Empty Entity")) {
                m_Scene->CreateEntity("Empty Entity");
            }
            ImGui::EndPopup();
        }

        ImGui::End();

        // --- Inspector ---
        ImGui::Begin("Inspector");
        if (m_SelectedEntity != entt::null) {
            auto& registry = m_Scene->Registry();

            // Tag Component
            if (registry.all_of<ECS::TagComponent>(m_SelectedEntity)) {
                auto& tag = registry.get<ECS::TagComponent>(m_SelectedEntity);
                char buffer[256];
                memset(buffer, 0, sizeof(buffer));
                strcpy_s(buffer, sizeof(buffer), tag.Tag.c_str());
                if (ImGui::InputText("Tag", buffer, sizeof(buffer))) {
                    tag.Tag = std::string(buffer);
                }
            }

            // Transform Component
            if (registry.all_of<ECS::TransformComponent>(m_SelectedEntity)) {
                if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                    auto& transform = registry.get<ECS::TransformComponent>(m_SelectedEntity);
                    
                    ImGui::DragFloat3("Translation", &transform.Translation.x, 0.1f);
                    
                    glm::vec3 rotation = glm::degrees(glm::eulerAngles(transform.Rotation));
                    if (ImGui::DragFloat3("Rotation", &rotation.x, 0.1f)) {
                        transform.Rotation = glm::quat(glm::radians(rotation));
                    }
                    
                    ImGui::DragFloat3("Scale", &transform.Scale.x, 0.1f);
                }
            }

            // RigidBody Component
            if (registry.all_of<ECS::RigidBodyComponent>(m_SelectedEntity)) {
                if (ImGui::CollapsingHeader("RigidBody", ImGuiTreeNodeFlags_DefaultOpen)) {
                    auto& rb = registry.get<ECS::RigidBodyComponent>(m_SelectedEntity);
                    
                    const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
                    int currentBodyType = static_cast<int>(rb.Type);
                    if (ImGui::Combo("Body Type", &currentBodyType, bodyTypeStrings, 3)) {
                        rb.Type = static_cast<ECS::BodyType>(currentBodyType);
                    }

                    ImGui::DragFloat("Mass", &rb.Mass, 0.1f, 0.0f, 1000.0f);
                    ImGui::DragFloat("Linear Drag", &rb.LinearDrag, 0.01f, 0.0f, 1.0f);
                    ImGui::DragFloat("Angular Drag", &rb.AngularDrag, 0.01f, 0.0f, 1.0f);
                    ImGui::Checkbox("Use Gravity", &rb.UseGravity);
                    ImGui::Checkbox("Is Sleeping", &rb.IsSleeping);
                    
                    ImGui::Text("Velocity: %.2f, %.2f, %.2f", rb.Velocity.x, rb.Velocity.y, rb.Velocity.z);
                }
            }

            // Collider Component
            if (registry.all_of<ECS::ColliderComponent>(m_SelectedEntity)) {
                if (ImGui::CollapsingHeader("Collider", ImGuiTreeNodeFlags_DefaultOpen)) {
                    auto& collider = registry.get<ECS::ColliderComponent>(m_SelectedEntity);
                    
                    const char* colliderTypeStrings[] = { "Box", "Sphere", "Capsule" };
                    int currentType = static_cast<int>(collider.Type);
                    if (ImGui::Combo("Type", &currentType, colliderTypeStrings, 3)) {
                        collider.Type = static_cast<ECS::ColliderType>(currentType);
                        // Reset shape variant when type changes
                        if (collider.Type == ECS::ColliderType::Box) collider.Shape = ECS::BoxCollider{};
                        else if (collider.Type == ECS::ColliderType::Sphere) collider.Shape = ECS::SphereCollider{};
                        else if (collider.Type == ECS::ColliderType::Capsule) collider.Shape = ECS::CapsuleCollider{};
                    }

                    if (collider.Type == ECS::ColliderType::Box) {
                        auto& box = std::get<ECS::BoxCollider>(collider.Shape);
                        ImGui::DragFloat3("Half Extents", &box.HalfExtents.x, 0.1f);
                        ImGui::DragFloat3("Offset", &box.Offset.x, 0.1f);
                    }
                    else if (collider.Type == ECS::ColliderType::Sphere) {
                        auto& sphere = std::get<ECS::SphereCollider>(collider.Shape);
                        ImGui::DragFloat("Radius", &sphere.Radius, 0.1f);
                        ImGui::DragFloat3("Offset", &sphere.Offset.x, 0.1f);
                    }

                    ImGui::DragFloat("Friction", &collider.Friction, 0.01f, 0.0f, 1.0f);
                    ImGui::DragFloat("Bounciness", &collider.Bounciness, 0.01f, 0.0f, 1.0f);
                    ImGui::Checkbox("Is Trigger", &collider.IsTrigger);
                }
            }

            // Light Component
            if (registry.all_of<ECS::LightComponent>(m_SelectedEntity)) {
                if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen)) {
                    auto& light = registry.get<ECS::LightComponent>(m_SelectedEntity);
                    ImGui::ColorEdit3("Color", &light.LightData.color.x);
                    ImGui::DragFloat("Intensity", &light.LightData.intensity, 0.1f, 0.0f, 100.0f);
                    ImGui::Checkbox("Cast Shadows", &light.CastShadows);
                }
            }
            
            // Add Component Button
            if (ImGui::Button("Add Component")) {
                ImGui::OpenPopup("AddComponent");
            }

            if (ImGui::BeginPopup("AddComponent")) {
                if (!registry.all_of<ECS::RigidBodyComponent>(m_SelectedEntity)) {
                    if (ImGui::MenuItem("RigidBody")) {
                        registry.emplace<ECS::RigidBodyComponent>(m_SelectedEntity);
                        ImGui::CloseCurrentPopup();
                    }
                }
                if (!registry.all_of<ECS::ColliderComponent>(m_SelectedEntity)) {
                    if (ImGui::MenuItem("Collider")) {
                        registry.emplace<ECS::ColliderComponent>(m_SelectedEntity);
                        ImGui::CloseCurrentPopup();
                    }
                }
                ImGui::EndPopup();
            }

        }
        ImGui::End();

        // --- System Settings ---
        ImGui::Begin("System Settings");
        ImGui::Text("Physics");
        // TODO: Get PhysicsSystem instance to modify gravity
        ImGui::End();
    }

} // namespace Yamen
