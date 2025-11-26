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
     * @brief Multi-Camera Scene - Advanced camera techniques
     * 
     * Features:
     * - Split-screen rendering
     * - Picture-in-picture
     * - Multiple camera perspectives
     * - Camera switching
     * - Minimap view
     */
    class MultiCameraScene : public IScene {
    public:
        MultiCameraScene(Graphics::GraphicsDevice& device);
        ~MultiCameraScene() override;

        bool Initialize() override;
        void Update(float deltaTime) override;
        void Render() override;
        void RenderImGui() override;

        const char* GetName() const override { return "Multi-Camera Demo"; }

    private:
        void CreateMainCamera();
        void CreateTopDownCamera();
        void CreateFollowCamera();
        void RenderSplitScreen();
        void RenderPictureInPicture();

        Graphics::GraphicsDevice& m_Device;
        
        std::unique_ptr<ECS::Scene> m_Scene;
        std::unique_ptr<Graphics::Renderer3D> m_Renderer3D;
        std::unique_ptr<Graphics::Renderer2D> m_Renderer2D;
        
        std::shared_ptr<Graphics::Mesh> m_CubeMesh;
        std::unique_ptr<Graphics::Shader> m_Shader;
        std::shared_ptr<Graphics::Texture2D> m_WhiteTexture;

        entt::entity m_MainCamera = entt::null;
        entt::entity m_TopDownCamera = entt::null;
        entt::entity m_FollowCamera = entt::null;
        entt::entity m_PlayerCube = entt::null;
        
        enum class ViewMode {
            Single,
            SplitScreen,
            PictureInPicture
        };
        ViewMode m_ViewMode = ViewMode::Single;
    };

} // namespace Yamen
