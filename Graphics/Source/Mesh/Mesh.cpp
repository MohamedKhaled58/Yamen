#include "Graphics/Mesh/Mesh.h"
#include "Core/Logging/Logger.h"

namespace Yamen::Graphics {

    Mesh::Mesh(GraphicsDevice& device)
        : m_Device(device)
        , m_VertexCount(0)
        , m_IndexCount(0)
    {
    }

    Mesh::~Mesh() {
    }

    bool Mesh::Create(const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices) {
        m_VertexCount = static_cast<uint32_t>(vertices.size());
        m_IndexCount = static_cast<uint32_t>(indices.size());

        // Create vertex buffer
        m_VertexBuffer = std::make_unique<Buffer>(m_Device, BufferType::Vertex);
        if (!m_VertexBuffer->Create(
            vertices.data(),
            m_VertexCount * sizeof(Vertex3D),
            sizeof(Vertex3D),
            BufferUsage::Immutable))
        {
            YAMEN_CORE_ERROR("Failed to create mesh vertex buffer");
            return false;
        }

        // Create index buffer
        m_IndexBuffer = std::make_unique<Buffer>(m_Device, BufferType::Index);
        if (!m_IndexBuffer->Create(
            indices.data(),
            m_IndexCount * sizeof(uint32_t),
            sizeof(uint32_t),
            BufferUsage::Immutable))
        {
            YAMEN_CORE_ERROR("Failed to create mesh index buffer");
            return false;
        }

        YAMEN_CORE_TRACE("Mesh created (vertices: {}, indices: {})", m_VertexCount, m_IndexCount);
        return true;
    }

    bool Mesh::CreateWithSubMeshes(const std::vector<Vertex3D>& vertices, 
                                   const std::vector<uint32_t>& indices,
                                   const std::vector<SubMesh>& subMeshes) {
        // Create buffers normally
        if (!Create(vertices, indices)) {
            return false;
        }

        // Store submeshes
        m_SubMeshes = subMeshes;

        YAMEN_CORE_TRACE("Mesh created with {} submeshes", m_SubMeshes.size());
        return true;
    }

    void Mesh::Bind() {
        m_VertexBuffer->Bind();
        m_IndexBuffer->Bind();
    }

    void Mesh::Draw() {
        auto context = m_Device.GetContext();
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->DrawIndexed(m_IndexCount, 0, 0);
    }

    void Mesh::DrawSubMesh(uint32_t submeshIndex) {
        if (submeshIndex >= m_SubMeshes.size()) {
            YAMEN_CORE_WARN("Invalid submesh index: {} (total: {})", submeshIndex, m_SubMeshes.size());
            return;
        }

        const auto& submesh = m_SubMeshes[submeshIndex];
        auto context = m_Device.GetContext();
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->DrawIndexed(submesh.indexCount, submesh.startIndex, submesh.baseVertex);
    }

} // namespace Yamen::Graphics
