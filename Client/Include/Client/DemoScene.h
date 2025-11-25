#pragma once

#include "Graphics/Graphics.h"
#include <memory>

namespace Yamen {

    /**
     * @brief Demo scene for testing Phase 3 graphics features
     * 
     * This class contains all test/demo code for the graphics system.
     * Enable/disable with ENABLE_DEMO_SCENE in Config.h
     */
    class DemoScene {
    public:
        DemoScene(Graphics::GraphicsDevice& device);
        ~DemoScene();

        /**
         * @brief Initialize demo resources
         */
        bool Initialize();

        /**
         * @brief Update demo scene
         */
        void Update(float deltaTime);

        /**
         * @brief Render demo scene
         */
        void Render();

        /**
         * @brief Render ImGui controls
         */
        void RenderImGui();

        /**
         * @brief Get the 3D camera
         */
        Graphics::Camera3D* GetCamera3D() const { return m_Camera3D.get(); }

    private:
        void CreateTestMeshes();
        void CreateTestTextures();

        Graphics::GraphicsDevice& m_Device;

        // Renderers
        std::unique_ptr<Graphics::Renderer2D> m_Renderer2D;
        std::unique_ptr<Graphics::Renderer3D> m_Renderer3D;

        // Cameras
        std::unique_ptr<Graphics::Camera2D> m_Camera2D;
        std::unique_ptr<Graphics::Camera3D> m_Camera3D;

        // Test meshes
        std::unique_ptr<Graphics::Mesh> m_CubeMesh;
        std::unique_ptr<Graphics::Mesh> m_SphereMesh;
        std::unique_ptr<Graphics::Mesh> m_PlaneMesh;

        // Test textures
        std::unique_ptr<Graphics::Texture2D> m_TestTexture;

        // Demo state
        float m_Rotation;
        bool m_ShowWireframe;
        bool m_Show2D;
        bool m_Show3D = true;
        glm::vec3 m_LightDirection;
        glm::vec3 m_LightColor;
    };

} // namespace Yamen
