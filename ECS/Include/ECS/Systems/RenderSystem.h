#pragma once

#include "ECS/ISystem.h"
#include "ECS/Scene.h"
#include "Graphics/Renderer/Renderer3D.h"
#include "Graphics/Renderer/Renderer2D.h"
#include "Graphics/Lighting/ShadowMap.h"
#include <memory>

namespace Yamen::ECS {

    /**
     * @brief Professional rendering system with multi-pass support
     * 
     * Features:
     * - Shadow map generation
     * - Opaque/transparent separation
     * - 2D sprite rendering
     * - Camera management
     * - Batch optimization
     */
    class RenderSystem : public ISystem {
    public:
        RenderSystem(Graphics::GraphicsDevice& device, Graphics::Renderer3D* renderer3D, Graphics::Renderer2D* renderer2D);
        ~RenderSystem() override;

        // ISystem interface
        void OnInit(Scene* scene) override;
        void OnUpdate(Scene* scene, float deltaTime) override;
        void OnRender(Scene* scene) override;
        void OnShutdown(Scene* scene) override;
        int GetPriority() const override { return 1000; } // Render last
        const char* GetName() const override { return "RenderSystem"; }

        // Shadow mapping
        void EnableShadows(bool enable) { m_ShadowsEnabled = enable; }
        bool AreShadowsEnabled() const { return m_ShadowsEnabled; }

    private:
        void RenderShadowPass(Scene* scene, Graphics::Camera3D* camera);
        void RenderOpaquePass(Scene* scene, Graphics::Camera3D* camera);
        void RenderTransparentPass(Scene* scene, Graphics::Camera3D* camera);
        void Render2DPass(Scene* scene);

        Graphics::GraphicsDevice& m_Device;
        Graphics::Renderer3D* m_Renderer3D;
        Graphics::Renderer2D* m_Renderer2D;
        
        std::unique_ptr<Graphics::ShadowMap> m_ShadowMap;
        bool m_ShadowsEnabled;
        
        // Performance tracking
        int m_DrawCallsThisFrame = 0;
        float m_PerfLogTimer = 0.0f;
    };

} // namespace Yamen::ECS
