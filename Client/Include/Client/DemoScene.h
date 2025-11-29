#pragma once

#include "Client/IScene.h"
#include "Graphics/Graphics.h"
#include <memory>
#include <Core/Math/Math.h>

namespace Yamen {

    /**
     * @brief Demo scene for testing Phase 3 graphics features
     * 
     * This class contains all test/demo code for the graphics system.
     * Enable/disable with ENABLE_DEMO_SCENE in Config.h
     */
    class DemoScene : public IScene {
    public:
        DemoScene(Graphics::GraphicsDevice& device);
        ~DemoScene() override;

        /**
         * @brief Initialize demo resources
         */
        bool Initialize() override;

        /**
         * @brief Update demo scene
         */
        void Update(float deltaTime) override;

        /**
         * @brief Render demo scene
         */
        void Render() override;

        /**
         * @brief Render ImGui controls
         */
        void RenderImGui() override;

        const char* GetName() const override { return "Legacy Demo"; }

        /**
         * @brief Get the 3D camera
         */
        Graphics::Camera3D* GetCamera3D() const { return m_Camera3D.get(); }

    private:
        void CreateTestMeshes();
        void CreateTestTextures();
        void CreateTestMaterials();

        Graphics::GraphicsDevice& m_Device;

        // Renderers
        std::unique_ptr<Graphics::Renderer2D> m_Renderer2D;
        std::unique_ptr<Graphics::Renderer3D> m_Renderer3D;

        // Cameras
        std::unique_ptr<Graphics::Camera2D> m_Camera2D;
        std::unique_ptr<Graphics::Camera3D> m_Camera3D;

        // New systems  
        std::unique_ptr<Graphics::ShaderLibrary> m_ShaderLibrary;
        std::unique_ptr<Graphics::LightManager> m_LightManager;

        // Test meshes
        std::unique_ptr<Graphics::Mesh> m_CubeMesh;
        std::unique_ptr<Graphics::Mesh> m_SphereMesh;
        std::unique_ptr<Graphics::Mesh> m_PlaneMesh;

        // Test textures
        std::unique_ptr<Graphics::Texture2D> m_TestTexture;

        // Test materials
        std::unique_ptr<Graphics::Material> m_RedMaterial;
        std::unique_ptr<Graphics::Material> m_GreenMaterial;
        std::unique_ptr<Graphics::Material> m_BlueMaterial;

        // Lights
        Graphics::Light m_SunLight;
        Graphics::Light m_PointLight1;
        Graphics::Light m_PointLight2;

        // Demo state
        float m_Rotation;
        bool m_ShowWireframe;
        bool m_Show2D;
        bool m_Show3D = true;
        bool m_UseMaterials = true;
        Core::vec3 m_LightDirection;
        Core::vec3 m_LightColor;
    };

} // namespace Yamen
