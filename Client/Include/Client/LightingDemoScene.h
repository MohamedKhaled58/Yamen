#pragma once

#include "Client/IScene.h"
#include "Graphics/RHI/GraphicsDevice.h"
#include "Graphics/Renderer/Renderer3D.h"
#include "Graphics/Renderer/Renderer2D.h"
#include "Graphics/Mesh/Mesh.h"
#include "Graphics/Shader/Shader.h"
#include "Graphics/Material/Material.h"
#include "ECS/Scene.h"
#include <memory>
#include <entt/entt.hpp>

namespace Yamen {

    /**
     * @brief Lighting Demo Scene - Advanced lighting showcase
     * 
     * Features:
     * - Multiple light types (Directional, Point, Spot)
     * - Dynamic light movement
     * - Shadow casting
     * - Light color animation
     * - Day/night cycle simulation
     */
    class LightingDemoScene : public IScene {
    public:
        LightingDemoScene(Graphics::GraphicsDevice& device);
        ~LightingDemoScene() override;

        bool Initialize() override;
        void Update(float deltaTime) override;
        void Render() override;
        void RenderImGui() override;

        const char* GetName() const override { return "Lighting Demo"; }

    private:
        void CreateEnvironment();
        void CreateDirectionalLight();
        void CreatePointLights();
        void CreateSpotLight();
        void UpdateLights(float deltaTime);

        Graphics::GraphicsDevice& m_Device;
        
        std::unique_ptr<ECS::Scene> m_Scene;
        std::unique_ptr<Graphics::Renderer3D> m_Renderer3D;
        std::unique_ptr<Graphics::Renderer2D> m_Renderer2D;
        
        std::shared_ptr<Graphics::Mesh> m_CubeMesh;
        std::shared_ptr<Graphics::Mesh> m_SphereMesh;
        std::unique_ptr<Graphics::Shader> m_Shader;
        std::shared_ptr<Graphics::Texture2D> m_WhiteTexture;

        entt::entity m_DirectionalLight = entt::null;
        entt::entity m_SpotLight = entt::null;
        std::vector<entt::entity> m_PointLights;
        
        float m_TimeOfDay = 0.0f;
        bool m_AnimateLights = true;
    };

} // namespace Yamen
