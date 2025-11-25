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

    void Mesh::Bind() {
        m_VertexBuffer->Bind();
        m_IndexBuffer->Bind();
    }

    void Mesh::Draw() {
        auto context = m_Device.GetContext();
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->DrawIndexed(m_IndexCount, 0, 0);
    }

} // namespace Yamen::Graphics
