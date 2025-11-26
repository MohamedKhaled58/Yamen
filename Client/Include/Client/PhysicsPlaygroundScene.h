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
     * @brief Physics Playground Scene - Interactive physics demonstration
     * 
     * Features:
     * - Multiple object types (boxes, spheres)
     * - Different physics materials (bouncy, heavy, light)
     * - Interactive spawning
     * - Stacking and domino effects
     * - Real-time physics tuning
     */
    class PhysicsPlaygroundScene : public IScene {
    public:
        PhysicsPlaygroundScene(Graphics::GraphicsDevice& device);
        ~PhysicsPlaygroundScene() override;

        bool Initialize() override;
        void Update(float deltaTime) override;
        void Render() override;
        void RenderImGui() override;

        const char* GetName() const override { return "Physics Playground"; }

    private:
        void CreateGround();
        void CreatePyramid();
        void CreateDominoChain();
        void CreateBouncyBalls();
        void CreateHeavyBox();
        void SpawnRandomObject();

        Graphics::GraphicsDevice& m_Device;
        
        std::unique_ptr<ECS::Scene> m_Scene;
        std::unique_ptr<Graphics::Renderer3D> m_Renderer3D;
        std::unique_ptr<Graphics::Renderer2D> m_Renderer2D;
        
        std::shared_ptr<Graphics::Mesh> m_CubeMesh;
        std::shared_ptr<Graphics::Mesh> m_SphereMesh;
        std::unique_ptr<Graphics::Shader> m_Shader;
        std::shared_ptr<Graphics::Texture2D> m_WhiteTexture;

        entt::entity m_SelectedEntity = entt::null;
        float m_SpawnTimer = 0.0f;
        bool m_AutoSpawn = false;
    };

} // namespace Yamen
