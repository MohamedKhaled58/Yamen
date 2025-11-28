#include "ECS/Systems/RenderSystem.h"
#include "ECS/Components.h"
#include <Core/Logging/Logger.h>
#include <entt/entt.hpp>
#include <algorithm>

namespace Yamen::ECS {

    RenderSystem::RenderSystem(Graphics::GraphicsDevice& device,
        Graphics::Renderer3D* renderer3D,
        Graphics::Renderer2D* renderer2D)
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
        m_ShadowMap = std::make_unique<Graphics::ShadowMap>(m_Device, 2048, 2048);
        YAMEN_CORE_INFO("RenderSystem initialized with ShadowMap");
    }

    void RenderSystem::OnUpdate(Scene* scene, float deltaTime) {}

    void RenderSystem::OnRender(Scene* scene) {
        if (!scene || !m_Renderer3D) return;

        entt::registry& reg = scene->Registry();

        // ────────────────────────────────────────────
        // FIND PRIMARY CAMERA
        // ────────────────────────────────────────────
        Graphics::Camera3D* mainCamera = nullptr;

        {
            auto cameraView = reg.view<TransformComponent, CameraComponent>();
            for (auto entity : cameraView) {
                auto& cam = cameraView.get<CameraComponent>(entity);
                if (cam.Primary) {
                    mainCamera = &cam.Camera;
                    break;
                }
            }
        }

        if (!mainCamera) {
            YAMEN_CORE_WARN("No primary camera found in scene");
            return;
        }

        // ────────────────────────────────────────────
        // UPDATE CAMERA TRANSFORMS
        // ────────────────────────────────────────────
        {
            auto cameraView = reg.view<TransformComponent, CameraComponent>();
            for (auto entity : cameraView) {
                auto& transform = cameraView.get<TransformComponent>(entity);
                auto& cam = cameraView.get<CameraComponent>(entity);

                cam.Camera.SetPosition(transform.Translation);
                cam.Camera.SetRotation(transform.GetRotationEuler());
            }
        }

        // ────────────────────────────────────────────
        // RENDER PASSES
        // ────────────────────────────────────────────
        if (m_ShadowsEnabled && m_ShadowMap)
            RenderShadowPass(scene, mainCamera);

        RenderOpaquePass(scene, mainCamera);
        RenderTransparentPass(scene, mainCamera);
        Render2DPass(scene);
    }

    void RenderSystem::OnShutdown(Scene* scene) {
        m_ShadowMap.reset();
    }

    // =======================================================================
    // SHADOW PASS
    // =======================================================================

    void RenderSystem::RenderShadowPass(Scene* scene, Graphics::Camera3D* camera) {
        auto& reg = scene->Registry();

        // Find a directional light for shadows
        Graphics::Light* shadowLight = nullptr;
        auto lightView = reg.view<LightComponent>();
        for (auto entity : lightView) {
            auto& lightComp = lightView.get<LightComponent>(entity);
            if (lightComp.Active && 
                lightComp.CastShadows && 
                lightComp.LightData.type == Graphics::LightType::Directional) {
                shadowLight = &lightComp.LightData;
                break;
            }
        }

        if (!shadowLight) return;

        m_Renderer3D->BeginShadowPass(m_ShadowMap.get(), shadowLight);

        // OPTIMIZATION: Only render objects that cast shadows
        auto meshView = reg.view<TransformComponent, MeshComponent>();
        for (auto entity : meshView) {
            auto& transform = meshView.get<TransformComponent>(entity);
            auto& mesh = meshView.get<MeshComponent>(entity);

            // Skip non-visible, non-shadow-casting, or null meshes
            if (!mesh.Visible || !mesh.CastShadows || !mesh.Mesh) continue;

            m_Renderer3D->DrawMeshWithSubMeshes(
                mesh.Mesh.get(),
                transform.GetTransform()
			);
        }

        m_Renderer3D->EndShadowPass();
    }

    // =======================================================================
    // OPAQUE PASS
    // =======================================================================

    void RenderSystem::RenderOpaquePass(Scene* scene, Graphics::Camera3D* camera) {
        auto& reg = scene->Registry();
        m_Renderer3D->BeginScene(camera);

        // Submit lights
        auto lightView = reg.view<TransformComponent, LightComponent>();
        for (auto entity : lightView) {
            auto& transform = lightView.get<TransformComponent>(entity);
            auto& lightComp = lightView.get<LightComponent>(entity);

            if (!lightComp.Active) continue;

            lightComp.LightData.position = transform.Translation;

            if (lightComp.LightData.type == Graphics::LightType::Directional ||
                lightComp.LightData.type == Graphics::LightType::Spot) {
                lightComp.LightData.direction = transform.GetForward();
            }

            m_Renderer3D->SubmitLight(lightComp.LightData);
        }

        // Bind shadow map
        if (m_ShadowsEnabled && m_ShadowMap)
            m_ShadowMap->BindSRV(1);

        // OPTIMIZATION: Batch by material to reduce state changes
        struct MeshRenderData {
            entt::entity entity;
            Graphics::Mesh* mesh;
            Graphics::Material* material;
            glm::mat4 transform;
        };
        
        std::vector<MeshRenderData> renderQueue;
        renderQueue.reserve(200); // Pre-allocate for typical scene size
        
        // Collect all visible meshes
        auto meshView = reg.view<TransformComponent, MeshComponent>();
        for (auto entity : meshView) {
            auto& transform = meshView.get<TransformComponent>(entity);
            auto& mesh = meshView.get<MeshComponent>(entity);

            if (!mesh.Visible || !mesh.Mesh) continue;
            
            renderQueue.push_back({
                entity,
                mesh.Mesh.get(),
                mesh.Material.get(),
                transform.GetTransform()
            });
        }
        
        // Sort by material pointer to batch same materials together
        std::sort(renderQueue.begin(), renderQueue.end(),
            [](const MeshRenderData& a, const MeshRenderData& b) {
                return a.material < b.material;
            });
        
        // Render batched meshes
        for (const auto& data : renderQueue) {
            m_Renderer3D->DrawMesh(data.mesh, data.transform, data.material);
        }

        m_Renderer3D->EndScene();
    }

    // =======================================================================
    // TRANSPARENT PASS
    // =======================================================================

    void RenderSystem::RenderTransparentPass(Scene* scene, Graphics::Camera3D* camera) {
        if (!camera) return;
        auto& reg = scene->Registry();

        struct TransparentItem {
            entt::entity entity;
            float distanceSq;
        };

        std::vector<TransparentItem> transparentList;

        auto meshView = reg.view<TransformComponent, MeshComponent>();
        glm::vec3 camPos = camera->GetPosition();

        for (auto entity : meshView) {
            auto& transform = meshView.get<TransformComponent>(entity);
            auto& mesh = meshView.get<MeshComponent>(entity);

            if (!mesh.Visible || !mesh.Mesh || !mesh.Material) continue;
            if (!mesh.Material->GetBlendState()) continue;

            float distSq = glm::distance2(transform.Translation, camPos);
            transparentList.push_back({ entity, distSq });
        }

        // Sort back-to-front
        std::sort(transparentList.begin(), transparentList.end(),
            [](const TransparentItem& a, const TransparentItem& b) {
                return a.distanceSq > b.distanceSq;
            });

        // Render
        for (auto& item : transparentList) {
            auto& transform = meshView.get<TransformComponent>(item.entity);
            auto& mesh = meshView.get<MeshComponent>(item.entity);

            m_Renderer3D->DrawMesh(
                mesh.Mesh.get(),
                transform.GetTransform(),
                mesh.Material.get()
            );
        }
    }

    // =======================================================================
    // 2D PASS
    // =======================================================================

    void RenderSystem::Render2DPass(Scene* scene) {
        if (!m_Renderer2D) return;

        auto& reg = scene->Registry();
        Graphics::Camera2D camera2D(1280.0f, 720.0f);
        m_Renderer2D->BeginScene(&camera2D);

        auto spriteView = reg.view<TransformComponent, SpriteComponent>();
        std::vector<entt::entity> sorted;

        for (auto entity : spriteView)
            sorted.push_back(entity);

        std::sort(sorted.begin(), sorted.end(), [&](entt::entity a, entt::entity b) {
            auto& sa = spriteView.get<SpriteComponent>(a);
            auto& sb = spriteView.get<SpriteComponent>(b);
            if (sa.SortingLayer != sb.SortingLayer)
                return sa.SortingLayer < sb.SortingLayer;
            return sa.OrderInLayer < sb.OrderInLayer;
            });

        for (auto entity : sorted) {
            auto& transform = spriteView.get<TransformComponent>(entity);
            auto& sprite = spriteView.get<SpriteComponent>(entity);

            glm::vec2 pos(transform.Translation.x, transform.Translation.y);
            glm::vec2 scale(transform.Scale.x, transform.Scale.y);

            if (sprite.Texture) {
                m_Renderer2D->DrawSprite(
                    sprite.Texture.get(),
                    pos,
                    scale,
                    transform.Rotation.z,
                    sprite.Color
                );
            }
            else {
                m_Renderer2D->DrawQuad(
                    pos,
                    scale,
                    sprite.Color,
                    transform.Rotation.z
                );
            }
        }

        m_Renderer2D->EndScene();
    }

} // namespace Yamen::ECS
