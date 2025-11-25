#pragma once

#include "Graphics/RHI/GraphicsDevice.h"
#include "Graphics/Renderer/Camera2D.h"
#include "Graphics/Renderer/SpriteBatch.h"
#include "Graphics/Texture/Texture2D.h"
#include "Graphics/Shader/Shader.h"
#include "Graphics/RHI/BlendState.h"
#include "Graphics/RHI/Sampler.h"
#include <glm/glm.hpp>
#include <memory>

namespace Yamen::Graphics {

    /**
     * @brief High-level 2D rendering API
     */
    class Renderer2D {
    public:
        Renderer2D(GraphicsDevice& device);
        ~Renderer2D();

        // Non-copyable
        Renderer2D(const Renderer2D&) = delete;
        Renderer2D& operator=(const Renderer2D&) = delete;

        /**
         * @brief Initialize renderer
         */
        bool Initialize();

        /**
         * @brief Begin 2D scene
         */
        void BeginScene(Camera2D* camera);

        /**
         * @brief End 2D scene
         */
        void EndScene();

        /**
         * @brief Draw sprite
         */
        void DrawSprite(
            Texture2D* texture,
            const glm::vec2& position,
            const glm::vec2& size = glm::vec2(1.0f),
            float rotation = 0.0f,
            const glm::vec4& color = glm::vec4(1.0f),
            const glm::vec2& origin = glm::vec2(0.0f)
        );

        /**
         * @brief Draw quad (no texture)
         */
        void DrawQuad(
            const glm::vec2& position,
            const glm::vec2& size,
            const glm::vec4& color,
            float rotation = 0.0f
        );

        /**
         * @brief Set blend mode
         */
        void SetBlendMode(BlendMode mode);

    private:
        GraphicsDevice& m_Device;
        std::unique_ptr<SpriteBatch> m_SpriteBatch;
        std::unique_ptr<Shader> m_Shader;
        std::unique_ptr<BlendState> m_BlendState;
        std::unique_ptr<Sampler> m_Sampler;
        std::unique_ptr<Texture2D> m_WhiteTexture;
        Camera2D* m_CurrentCamera;
        bool m_InScene;
    };

} // namespace Yamen::Graphics
