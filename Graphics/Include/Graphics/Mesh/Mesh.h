#pragma once

#include "Graphics/RHI/GraphicsDevice.h"
#include "Graphics/RHI/Buffer.h"
#include "Graphics/Mesh/Vertex.h"
#include "Graphics/Mesh/SubMesh.h"
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
         * @brief Create mesh with multiple sub-meshes (multi-material support)
         */
        bool CreateWithSubMeshes(const std::vector<Vertex3D>& vertices, 
                                const std::vector<uint32_t>& indices,
                                const std::vector<SubMesh>& subMeshes);

        /**
         * @brief Bind mesh for rendering
         */
        void Bind();

        /**
         * @brief Draw mesh (single material)
         */
        void Draw();

        /**
         * @brief Draw specific submesh
         */
        void DrawSubMesh(uint32_t submeshIndex);

        /**
         * @brief Get vertex count
         */
        uint32_t GetVertexCount() const { return m_VertexCount; }

        /**
         * @brief Get index count
         */
        uint32_t GetIndexCount() const { return m_IndexCount; }

        /**
         * @brief Get submeshes
         */
        const std::vector<SubMesh>& GetSubMeshes() const { return m_SubMeshes; }
        bool HasSubMeshes() const { return !m_SubMeshes.empty(); }

    private:
        GraphicsDevice& m_Device;
        std::unique_ptr<Buffer> m_VertexBuffer;
        std::unique_ptr<Buffer> m_IndexBuffer;
        uint32_t m_VertexCount;
        uint32_t m_IndexCount;
        std::vector<SubMesh> m_SubMeshes;
    };

} // namespace Yamen::Graphics
