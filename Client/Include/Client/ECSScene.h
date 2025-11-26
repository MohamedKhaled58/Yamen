#pragma once

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
     * @brief ECS-based demo scene
     * 
     * Demonstrates the new professional ECS architecture with:
     * - System-based updates
     * - Component-based entities
     * - Native script support
     * - Multi-pass rendering
     */
    class ECSScene {
    public:
        ECSScene(Graphics::GraphicsDevice& device);
        ~ECSScene();

        bool Initialize();
        void Update(float deltaTime);
        void Render();
        void RenderImGui();

    private:
        Graphics::GraphicsDevice& m_Device;
        
        // Scene and systems
        std::unique_ptr<ECS::Scene> m_Scene;
        std::unique_ptr<Graphics::Renderer3D> m_Renderer3D;
        std::unique_ptr<Graphics::Renderer2D> m_Renderer2D;
        
        // Resources
        std::shared_ptr<Graphics::Mesh> m_CubeMesh;
        std::unique_ptr<Graphics::Shader> m_Shader;
        std::shared_ptr<Graphics::Material> m_Material;
        std::shared_ptr<Graphics::Texture2D> m_WhiteTexture; // White texture for materials

        // Editor State
        entt::entity m_SelectedEntity = entt::null;
    };

} // namespace Yamen
