#include "ECS/Systems/RenderSystem.h"
#include "ECS/Components.h"
#include <Core/Logging/Logger.h>
#include <entt/entt.hpp>
#include <algorithm>

namespace Yamen::ECS {

    RenderSystem::RenderSystem(Graphics::GraphicsDevice& device, Graphics::Renderer3D* renderer3D, Graphics::Renderer2D* renderer2D)
        : m_Device(device)
        , m_Renderer3D(renderer3D)
        , m_Renderer2D(renderer2D)
        , m_ShadowsEnabled(true)
    {
        YAMEN_CORE_INFO("RenderSystem created");
    }

    RenderSystem::~RenderSystem() {
        YAMEN_CORE_INFO("RenderSystem destroyed");
    }

    void RenderSystem::OnInit(Scene* scene) {
        // Create shadow map (2048x2048 resolution)
        m_ShadowMap = std::make_unique<Graphics::ShadowMap>(m_Device, 2048, 2048);
        YAMEN_CORE_INFO("RenderSystem initialized with ShadowMap");
    }

    void RenderSystem::OnUpdate(Scene* scene, float deltaTime) {
        // Animation or pre-render updates could go here
    }

    void RenderSystem::OnRender(Scene* scene) {
        if (!scene || !m_Renderer3D) return;

        // Find primary camera
        Graphics::Camera3D* mainCamera = nullptr;
        auto cameraView = scene->Registry().view<TransformComponent, CameraComponent>();
        
        // Sort by render order and find primary
        std::vector<std::pair<entt::entity, CameraComponent*>> cameras;
        for (auto entity : cameraView) {
            auto& camera = cameraView.get<CameraComponent>(entity);
            if (camera.Primary) {
                mainCamera = &camera.Camera;
                break;
            }
        }

        if (!mainCamera) {
            YAMEN_CORE_WARN("No primary camera found in scene");
            return;
        }

        // Update camera transforms
        for (auto entity : cameraView) {
            auto [transform, camera] = cameraView.get<TransformComponent, CameraComponent>(entity);
            camera.Camera.SetPosition(transform.Translation);
            camera.Camera.SetRotation(transform.GetRotationEuler());
        }

        // Debug: Log camera info once
        static bool logged = false;
        if (!logged) {
            auto pos = mainCamera->GetPosition();
            auto rot = mainCamera->GetRotation();
            YAMEN_CORE_INFO("Camera: pos({}, {}, {}), rot({}, {}, {}), fov={}", 
                pos.x, pos.y, pos.z, rot.x, rot.y, rot.z, mainCamera->GetFOV());
            logged = true;
        }

        // Multi-pass rendering
        if (m_ShadowsEnabled && m_ShadowMap) {
            RenderShadowPass(scene, mainCamera);
        }
        
        RenderOpaquePass(scene, mainCamera);
        RenderTransparentPass(scene, mainCamera);
        Render2DPass(scene);
    }

    void RenderSystem::OnShutdown(Scene* scene) {
        m_ShadowMap.reset();
    }

    void RenderSystem::RenderShadowPass(Scene* scene, Graphics::Camera3D* camera) {
        // Find directional light for shadow casting
        auto lightView = scene->Registry().view<TransformComponent, LightComponent>();
        Graphics::Light* shadowLight = nullptr;
        
        for (auto entity : lightView) {
            auto [transform, light] = lightView.get<TransformComponent, LightComponent>(entity);
            if (light.Active && light.CastShadows && 
                light.LightData.type == Graphics::LightType::Directional) {
                shadowLight = &light.LightData;
                break;
            }
        }

        if (!shadowLight) return;

        // Begin shadow pass
        m_Renderer3D->BeginShadowPass(m_ShadowMap.get(), shadowLight);

        // Render shadow-casting meshes
        auto meshView = scene->Registry().view<TransformComponent, MeshComponent>();
        for (auto entity : meshView) {
            auto [transform, mesh] = meshView.get<TransformComponent, MeshComponent>(entity);
            if (mesh.Visible && mesh.CastShadows && mesh.Mesh) {
                m_Renderer3D->DrawMesh(mesh.Mesh.get(), transform.GetTransform(), static_cast<Graphics::Material*>(nullptr));
            }
        }

        m_Renderer3D->EndShadowPass();
    }

    void RenderSystem::RenderOpaquePass(Scene* scene, Graphics::Camera3D* camera) {
        m_Renderer3D->BeginScene(camera);

        // Submit lights
        auto lightView = scene->Registry().view<TransformComponent, LightComponent>();
        int lightCount = 0;
        for (auto entity : lightView) {
            auto [transform, light] = lightView.get<TransformComponent, LightComponent>(entity);
            if (light.Active) {
                // Update light position from transform
                light.LightData.position = transform.Translation;
                
                // Update direction for directional/spot lights
                if (light.LightData.type == Graphics::LightType::Directional ||
                    light.LightData.type == Graphics::LightType::Spot) {
                    light.LightData.direction = transform.GetForward();
                }
                
                m_Renderer3D->SubmitLight(light.LightData);
                lightCount++;
            }
        }

        // Bind shadow map if available
        if (m_ShadowsEnabled && m_ShadowMap) {
            m_ShadowMap->BindSRV(1); // Bind to slot 1
        }

        // Render opaque meshes
        auto meshView = scene->Registry().view<TransformComponent, MeshComponent>();
        int meshCount = 0;
        for (auto entity : meshView) {
            auto [transform, mesh] = meshView.get<TransformComponent, MeshComponent>(entity);
            if (mesh.Visible && mesh.Mesh) {
                // Debug: Log first mesh position once
                static bool meshLogged = false;
                if (!meshLogged) {
                    auto pos = transform.Translation;
                    YAMEN_CORE_INFO("Mesh: pos({}, {}, {}), visible={}, hasMaterial={}", 
                        pos.x, pos.y, pos.z, mesh.Visible, mesh.Material != nullptr);
                    meshLogged = true;
                }
                
                m_Renderer3D->DrawMesh(
                    mesh.Mesh.get(),
                    transform.GetTransform(),
                    mesh.Material.get()
                );
                meshCount++;
            }
        }

        YAMEN_CORE_INFO("Rendered {} lights, {} meshes", lightCount, meshCount);
        m_Renderer3D->EndScene();
    }

    void RenderSystem::RenderTransparentPass(Scene* scene, Graphics::Camera3D* camera) {
        if (!camera) return;

        // Collect transparent meshes
        struct TransparentMesh {
            entt::entity Entity;
            float DistanceSq;
        };
        std::vector<TransparentMesh> transparentMeshes;

        auto meshView = scene->Registry().view<TransformComponent, MeshComponent>();
        glm::vec3 cameraPos = camera->GetPosition();

        for (auto entity : meshView) {
            auto [transform, mesh] = meshView.get<TransformComponent, MeshComponent>(entity);
            if (mesh.Visible && mesh.Mesh && mesh.Material && mesh.Material->GetBlendState()) {
                float distSq = glm::distance2(transform.Translation, cameraPos);
                transparentMeshes.push_back({ entity, distSq });
            }
        }

        // Sort back-to-front
        std::sort(transparentMeshes.begin(), transparentMeshes.end(),
            [](const TransparentMesh& a, const TransparentMesh& b) {
                return a.DistanceSq > b.DistanceSq;
            });

        // Render
        for (const auto& tm : transparentMeshes) {
            auto [transform, mesh] = meshView.get<TransformComponent, MeshComponent>(tm.Entity);
            m_Renderer3D->DrawMesh(
                mesh.Mesh.get(),
                transform.GetTransform(),
                mesh.Material.get()
            );
        }
    }

    void RenderSystem::Render2DPass(Scene* scene) {
        if (!m_Renderer2D) return;

        // Find 2D camera or use default
        // For now, we'll just use a default camera if none is found
        // TODO: Implement proper 2D Camera Component lookup
        // Initialize with default resolution (1280x720)
        Graphics::Camera2D camera2D(1280.0f, 720.0f);
        
        m_Renderer2D->BeginScene(&camera2D);

        // Render sprites sorted by layer and order
        auto spriteView = scene->Registry().view<TransformComponent, SpriteComponent>();
        
        // Sort sprites by layer and order
        std::vector<entt::entity> sortedSprites;
        for (auto entity : spriteView) {
            sortedSprites.push_back(entity);
        }
        
        std::sort(sortedSprites.begin(), sortedSprites.end(),
            [&](entt::entity a, entt::entity b) {
                auto& spriteA = spriteView.get<SpriteComponent>(a);
                auto& spriteB = spriteView.get<SpriteComponent>(b);
                if (spriteA.SortingLayer != spriteB.SortingLayer)
                    return spriteA.SortingLayer < spriteB.SortingLayer;
                return spriteA.OrderInLayer < spriteB.OrderInLayer;
            });

        // Render sorted sprites
        for (auto entity : sortedSprites) {
            auto [transform, sprite] = spriteView.get<TransformComponent, SpriteComponent>(entity);
            
            if (sprite.Texture) {
                m_Renderer2D->DrawSprite(
                    sprite.Texture.get(),
                    glm::vec2(transform.Translation.x, transform.Translation.y),
                    glm::vec2(transform.Scale.x, transform.Scale.y),
                    transform.Rotation.z, // 2D rotation usually around Z
                    sprite.Color
                );
            } else {
                m_Renderer2D->DrawQuad(
                    glm::vec2(transform.Translation.x, transform.Translation.y),
                    glm::vec2(transform.Scale.x, transform.Scale.y),
                    sprite.Color,
                    transform.Rotation.z
                );
            }
        }
        
        m_Renderer2D->EndScene();
    }

} // namespace Yamen::ECS
