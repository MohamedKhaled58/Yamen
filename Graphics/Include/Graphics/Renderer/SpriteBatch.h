#pragma once

#include "Graphics/RHI/GraphicsDevice.h"
#include "Graphics/RHI/Buffer.h"
#include "Graphics/RHI/InputLayout.h"
#include "Graphics/Mesh/Vertex.h"
#include "Graphics/Texture/Texture2D.h"
#include "Graphics/Shader/Shader.h"
#include "Graphics/Renderer/Camera2D.h"
#include <glm/glm.hpp>
#include <vector>

namespace Yamen::Graphics {

    /**
     * @brief Batched 2D sprite renderer
     */
    class SpriteBatch {
    public:
        SpriteBatch(GraphicsDevice& device, uint32_t maxSprites = 10000);
        ~SpriteBatch();

        // Non-copyable
        SpriteBatch(const SpriteBatch&) = delete;
        SpriteBatch& operator=(const SpriteBatch&) = delete;

        /**
         * @brief Initialize sprite batch
         */
        bool Initialize();

        /**
         * @brief Begin sprite batch
         */
        void Begin(Camera2D* camera);

        /**
         * @brief Draw sprite
         */
        void DrawSprite(
            Texture2D* texture,
            const glm::vec2& position,
            const glm::vec2& size,
            float rotation = 0.0f,
            const glm::vec4& color = glm::vec4(1.0f),
            const glm::vec2& origin = glm::vec2(0.0f)
        );

        /**
         * @brief End sprite batch and flush
         */
        void End();

    private:
        void Flush();
        void EnsureCapacity();

        GraphicsDevice& m_Device;
        std::unique_ptr<Buffer> m_VertexBuffer;
        std::unique_ptr<Buffer> m_IndexBuffer;
        std::unique_ptr<Shader> m_Shader;
        std::unique_ptr<Buffer> m_ConstantBuffer;  // Camera matrix constant buffer
        std::unique_ptr<InputLayout> m_InputLayout;  // Vertex2D input layout

        std::vector<Vertex2D> m_Vertices;
        std::vector<uint32_t> m_Indices;

        uint32_t m_MaxSprites;
        uint32_t m_SpriteCount;
        Texture2D* m_CurrentTexture;
        Camera2D* m_Camera;
        bool m_InBatch;
    };

} // namespace Yamen::Graphics
