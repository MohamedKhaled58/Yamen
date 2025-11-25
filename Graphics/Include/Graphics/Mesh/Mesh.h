#pragma once

#include "Graphics/RHI/GraphicsDevice.h"
#include "Graphics/RHI/Buffer.h"
#include "Graphics/Mesh/Vertex.h"
#include <vector>
#include <glm/glm.hpp>

namespace Yamen::Graphics {

    /**
     * @brief 3D mesh representation
     */
    class Mesh {
    public:
        Mesh(GraphicsDevice& device);
        ~Mesh();

        // Non-copyable
        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;

        /**
         * @brief Create mesh from vertices and indices
         */
        bool Create(const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices);

        /**
         * @brief Bind mesh for rendering
         */
        void Bind();

        /**
         * @brief Draw mesh
         */
        void Draw();

        /**
         * @brief Get vertex count
         */
        uint32_t GetVertexCount() const { return m_VertexCount; }

        /**
         * @brief Get index count
         */
        uint32_t GetIndexCount() const { return m_IndexCount; }

    private:
        GraphicsDevice& m_Device;
        std::unique_ptr<Buffer> m_VertexBuffer;
        std::unique_ptr<Buffer> m_IndexBuffer;
        uint32_t m_VertexCount;
        uint32_t m_IndexCount;
    };

} // namespace Yamen::Graphics
