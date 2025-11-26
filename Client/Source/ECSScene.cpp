#include "Client/ECSScene.h"
#include "Client/CameraController.h"
#include "ECS/Components.h"
#include "ECS/Systems/RenderSystem.h"
#include "ECS/Systems/CameraSystem.h"
#include "ECS/Systems/PhysicsSystem.h"
#include "ECS/Systems/ScriptSystem.h"
#include "ECS/Systems/GizmoSystem.h"
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
        m_Scene = std::make_unique<ECS::Scene>("Main Scene");
        
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

        // Add systems - GizmoSystem is now part of ECS!
        m_Scene->AddSystem<ECS::CameraSystem>();
        m_Scene->AddSystem<ECS::ScriptSystem>();
        m_Scene->AddSystem<ECS::PhysicsSystem>();
        m_Scene->AddSystem<ECS::GizmoSystem>(); // Dynamic gizmo system
        m_Scene->AddSystem<ECS::RenderSystem>(m_Device, m_Renderer3D.get(), m_Renderer2D.get());
        m_Scene->OnInit();

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
        
        m_Material = std::make_shared<Graphics::Material>();
        m_Material->SetShader(m_Shader.get());
        m_Material->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_WhiteTexture.get());
        m_Material->SetVector(Graphics::Material::ALBEDO_COLOR, glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));

        // Camera - FIXED: Identity rotation
        auto cameraEntity = m_Scene->CreateEntity("MainCamera");
        auto& cameraComp = cameraEntity.AddComponent<ECS::CameraComponent>();
        cameraComp.Primary = true;
        cameraComp.Camera.SetFOV(60.0f);
        cameraComp.Camera.SetAspectRatio(16.0f/9.0f);
        cameraComp.Camera.SetClipPlanes(0.1f, 1000.0f);
        
        auto& cameraTransform = cameraEntity.GetComponent<ECS::TransformComponent>();
        cameraTransform.Translation = glm::vec3(0.0f, 10.0f, -20.0f);
        cameraTransform.Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        
        auto& script = cameraEntity.AddComponent<ECS::NativeScriptComponent>();
        script.Bind<Client::CameraController>();

        CreateCoordinateGizmos();
        CreatePhysicsDemo();

        auto lightEntity = m_Scene->CreateEntity("Sun");
        auto& lightComp = lightEntity.AddComponent<ECS::LightComponent>();
        lightComp.LightData = Graphics::Light::CreateDirectional(
            glm::normalize(glm::vec3(-0.3f, -1.0f, -0.3f)),
            glm::vec3(1.0f, 1.0f, 1.0f),
            3.0f
        );
        lightComp.Active = true;
        lightComp.CastShadows = false;

        YAMEN_CORE_INFO("ECS Scene Initialized with GizmoSystem");
        return true;
    }

    void ECSScene::CreateCoordinateGizmos() {
        auto axisX = m_Scene->CreateEntity("Axis_X");
        auto& meshAxisX = axisX.AddComponent<ECS::MeshComponent>();
        meshAxisX.Mesh = m_CubeMesh;
        auto matAxisX = std::make_shared<Graphics::Material>();
        matAxisX->SetShader(m_Shader.get());
        matAxisX->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_WhiteTexture.get());
        matAxisX->SetVector(Graphics::Material::ALBEDO_COLOR, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        meshAxisX.Material = matAxisX;
        auto& transX = axisX.GetComponent<ECS::TransformComponent>();
        transX.Translation = glm::vec3(5.0f, 0.0f, 0.0f);
        transX.Scale = glm::vec3(10.0f, 0.2f, 0.2f);

        auto axisY = m_Scene->CreateEntity("Axis_Y");
        auto& meshAxisY = axisY.AddComponent<ECS::MeshComponent>();
        meshAxisY.Mesh = m_CubeMesh;
        auto matAxisY = std::make_shared<Graphics::Material>();
        matAxisY->SetShader(m_Shader.get());
        matAxisY->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_WhiteTexture.get());
        matAxisY->SetVector(Graphics::Material::ALBEDO_COLOR, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
        meshAxisY.Material = matAxisY;
        auto& transY = axisY.GetComponent<ECS::TransformComponent>();
        transY.Translation = glm::vec3(0.0f, 5.0f, 0.0f);
        transY.Scale = glm::vec3(0.2f, 10.0f, 0.2f);

        auto axisZ = m_Scene->CreateEntity("Axis_Z");
        auto& meshAxisZ = axisZ.AddComponent<ECS::MeshComponent>();
        meshAxisZ.Mesh = m_CubeMesh;
        auto matAxisZ = std::make_shared<Graphics::Material>();
        matAxisZ->SetShader(m_Shader.get());
        matAxisZ->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_WhiteTexture.get());
        matAxisZ->SetVector(Graphics::Material::ALBEDO_COLOR, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
        meshAxisZ.Material = matAxisZ;
        auto& transZ = axisZ.GetComponent<ECS::TransformComponent>();
        transZ.Translation = glm::vec3(0.0f, 0.0f, 5.0f);
        transZ.Scale = glm::vec3(0.2f, 0.2f, 10.0f);
    }

    void ECSScene::CreatePhysicsDemo() {
        auto ground = m_Scene->CreateEntity("Ground");
        auto& groundMesh = ground.AddComponent<ECS::MeshComponent>();
        groundMesh.Mesh = m_CubeMesh;
        auto groundMat = std::make_shared<Graphics::Material>();
        groundMat->SetShader(m_Shader.get());
        groundMat->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_WhiteTexture.get());
        groundMat->SetVector(Graphics::Material::ALBEDO_COLOR, glm::vec4(0.3f, 0.3f, 0.3f, 1.0f));
        groundMesh.Material = groundMat;
        
        auto& groundTrans = ground.GetComponent<ECS::TransformComponent>();
        groundTrans.Translation = glm::vec3(0.0f, -0.5f, 0.0f);
        groundTrans.Scale = glm::vec3(50.0f, 1.0f, 50.0f);
        
        auto& groundRb = ground.AddComponent<ECS::RigidBodyComponent>();
        groundRb.Type = ECS::BodyType::Static;
        
        ECS::BoxCollider groundBox;
        groundBox.HalfExtents = glm::vec3(25.0f, 0.5f, 25.0f);
        ground.AddComponent<ECS::ColliderComponent>(groundBox);

        auto testCube = m_Scene->CreateEntity("TestCube_Origin");
        auto& testMesh = testCube.AddComponent<ECS::MeshComponent>();
        testMesh.Mesh = m_CubeMesh;
        auto testMat = std::make_shared<Graphics::Material>();
        testMat->SetShader(m_Shader.get());
        testMat->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_WhiteTexture.get());
        testMat->SetVector(Graphics::Material::ALBEDO_COLOR, glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));
        testMesh.Material = testMat;
        
        auto& testTrans = testCube.GetComponent<ECS::TransformComponent>();
        testTrans.Translation = glm::vec3(0.0f, 1.0f, 0.0f);
        testTrans.Scale = glm::vec3(2.0f);

        auto physBox = m_Scene->CreateEntity("PhysicsBox");
        auto& physBoxMesh = physBox.AddComponent<ECS::MeshComponent>();
        physBoxMesh.Mesh = m_CubeMesh;
        auto physBoxMat = std::make_shared<Graphics::Material>();
        physBoxMat->SetShader(m_Shader.get());
        physBoxMat->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_WhiteTexture.get());
        physBoxMat->SetVector(Graphics::Material::ALBEDO_COLOR, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
        physBoxMesh.Material = physBoxMat;
        
        auto& physBoxTrans = physBox.GetComponent<ECS::TransformComponent>();
        physBoxTrans.Translation = glm::vec3(-5.0f, 10.0f, 0.0f);
        physBoxTrans.Scale = glm::vec3(1.5f);
        
        auto& physBoxRb = physBox.AddComponent<ECS::RigidBodyComponent>();
        physBoxRb.Mass = 1.0f;
        physBoxRb.Type = ECS::BodyType::Dynamic;
        
        ECS::BoxCollider physBoxCol;
        physBoxCol.HalfExtents = glm::vec3(0.75f);
        physBox.AddComponent<ECS::ColliderComponent>(physBoxCol);

        auto testCube2 = m_Scene->CreateEntity("TestCube_Right");
        auto& test2Mesh = testCube2.AddComponent<ECS::MeshComponent>();
        test2Mesh.Mesh = m_CubeMesh;
        auto test2Mat = std::make_shared<Graphics::Material>();
        test2Mat->SetShader(m_Shader.get());
        test2Mat->SetTexture(Graphics::Material::DIFFUSE_TEXTURE, m_WhiteTexture.get());
        test2Mat->SetVector(Graphics::Material::ALBEDO_COLOR, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
        test2Mesh.Material = test2Mat;
        
        auto& test2Trans = testCube2.GetComponent<ECS::TransformComponent>();
        test2Trans.Translation = glm::vec3(5.0f, 1.0f, 0.0f);
        test2Trans.Scale = glm::vec3(2.0f);
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

        // Get GizmoSystem from ECS
        auto* gizmoSystem = m_Scene->GetSystem<ECS::GizmoSystem>();

        // Scene Hierarchy
        ImGui::Begin("Scene Hierarchy");
        ImGui::TextColored(ImVec4(1,1,0,1), "CONTROLS:");
        ImGui::Text("WASD = Move | RMB+Mouse = Look");
        ImGui::Text("Space = Up | Ctrl = Down | Shift = Fast");
        ImGui::Separator();
        
        if (gizmoSystem) {
            ImGui::TextColored(ImVec4(0,1,1,1), "GIZMO MODES:");
            ImGui::Text("W = Translate | E = Rotate | R = Scale");
            const char* modeNames[] = { "Translate", "Rotate", "Scale", "Universal" };
            int currentMode = (gizmoSystem->GetOperation() == ImGuizmo::TRANSLATE) ? 0 : 
                             (gizmoSystem->GetOperation() == ImGuizmo::ROTATE) ? 1 : 2;
            ImGui::Text("Current: %s", modeNames[currentMode]);
            ImGui::Separator();
        }
        
        auto view = m_Scene->Registry().view<ECS::TagComponent>();
        for (auto entity : view) {
            auto& tag = view.get<ECS::TagComponent>(entity);
            
            ImGuiTreeNodeFlags flags = ((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | 
                                      ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Leaf;
            bool opened = ImGui::TreeNodeEx((void*)(uint64_t)entity, flags, "%s", tag.Tag.c_str());
            
            if (ImGui::IsItemClicked()) {
                m_SelectedEntity = entity;
                if (gizmoSystem) {
                    gizmoSystem->SetSelectedEntity(entity);
                }
            }

            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Delete Entity")) {
                    m_Scene->Registry().destroy(entity);
                    if (m_SelectedEntity == entity) {
                        m_SelectedEntity = entt::null;
                        if (gizmoSystem) gizmoSystem->SetSelectedEntity(entt::null);
                    }
                }
                ImGui::EndPopup();
            }

            if (opened) {
                ImGui::TreePop();
            }
        }
        
        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
            m_SelectedEntity = entt::null;
            if (gizmoSystem) gizmoSystem->SetSelectedEntity(entt::null);
        }

        if (ImGui::BeginPopupContextWindow()) {
            if (ImGui::MenuItem("Create Empty Entity")) {
                m_Scene->CreateEntity("Empty Entity");
            }
            ImGui::EndPopup();
        }

        ImGui::End();

        // Inspector
        ImGui::Begin("Inspector");
        if (m_SelectedEntity != entt::null) {
            auto& registry = m_Scene->Registry();

            if (registry.all_of<ECS::TagComponent>(m_SelectedEntity)) {
                auto& tag = registry.get<ECS::TagComponent>(m_SelectedEntity);
                char buffer[256];
                memset(buffer, 0, sizeof(buffer));
                strcpy_s(buffer, sizeof(buffer), tag.Tag.c_str());
                if (ImGui::InputText("Tag", buffer, sizeof(buffer))) {
                    tag.Tag = std::string(buffer);
                }
            }

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

            if (registry.all_of<ECS::ColliderComponent>(m_SelectedEntity)) {
                if (ImGui::CollapsingHeader("Collider", ImGuiTreeNodeFlags_DefaultOpen)) {
                    auto& collider = registry.get<ECS::ColliderComponent>(m_SelectedEntity);
                    
                    const char* colliderTypeStrings[] = { "Box", "Sphere", "Capsule" };
                    int currentType = static_cast<int>(collider.Type);
                    if (ImGui::Combo("Type", &currentType, colliderTypeStrings, 3)) {
                        collider.Type = static_cast<ECS::ColliderType>(currentType);
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

            if (registry.all_of<ECS::LightComponent>(m_SelectedEntity)) {
                if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen)) {
                    auto& light = registry.get<ECS::LightComponent>(m_SelectedEntity);
                    ImGui::ColorEdit3("Color", &light.LightData.color.x);
                    ImGui::DragFloat("Intensity", &light.LightData.intensity, 0.1f, 0.0f, 100.0f);
                    ImGui::Checkbox("Cast Shadows", &light.CastShadows);
                }
            }
            
            if (ImGui::Button("Add Component")) {
                ImGui::OpenPopup("AddComponentPopup");
            }

            if (ImGui::BeginPopup("AddComponentPopup")) {
                if (!registry.all_of<ECS::RigidBodyComponent>(m_SelectedEntity)) {
                    if (ImGui::MenuItem("RigidBody")) {
                        registry.emplace<ECS::RigidBodyComponent>(m_SelectedEntity);
                    }
                }
                if (!registry.all_of<ECS::ColliderComponent>(m_SelectedEntity)) {
                    if (ImGui::MenuItem("Collider")) {
                        registry.emplace<ECS::ColliderComponent>(m_SelectedEntity);
                    }
                }
                if (!registry.all_of<ECS::LightComponent>(m_SelectedEntity)) {
                    if (ImGui::MenuItem("Light")) {
                        registry.emplace<ECS::LightComponent>(m_SelectedEntity);
                    }
                }
                ImGui::EndPopup();
            }
        }
        ImGui::End();

        // System Settings
        ImGui::Begin("System Settings");
        
        if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto* physicsSystem = m_Scene->GetSystem<ECS::PhysicsSystem>();
            if (physicsSystem) {
                ImGui::DragFloat3("Gravity", &physicsSystem->Gravity.x, 0.1f, -50.0f, 50.0f);
                ImGui::DragInt("SubSteps", &physicsSystem->SubSteps, 1, 1, 10);
            }
        }
        
        if (gizmoSystem && ImGui::CollapsingHeader("Gizmo Settings")) {
            bool useSnap = gizmoSystem->IsSnapEnabled();
            if (ImGui::Checkbox("Use Snap", &useSnap)) {
                gizmoSystem->SetSnapEnabled(useSnap);
            }
            
            if (useSnap) {
                float snap[3];
                // Get current snap values (would need getter in GizmoSystem)
                ImGui::DragFloat3("Snap Values", snap, 0.1f, 0.1f, 10.0f);
                gizmoSystem->SetSnapValues(snap[0], snap[1], snap[2]);
            }
            
            const char* modeStrings[] = { "World", "Local" };
            int currentModeIdx = (gizmoSystem->GetMode() == ImGuizmo::WORLD) ? 0 : 1;
            if (ImGui::Combo("Gizmo Space", &currentModeIdx, modeStrings, 2)) {
                gizmoSystem->SetMode((currentModeIdx == 0) ? ImGuizmo::WORLD : ImGuizmo::LOCAL);
            }
        }
        
        ImGui::End();

        // Render gizmo AFTER all ImGui windows (important!)
        if (gizmoSystem) {
            gizmoSystem->RenderGizmo(m_Scene.get());
        }
    }

} // namespace Yamen
