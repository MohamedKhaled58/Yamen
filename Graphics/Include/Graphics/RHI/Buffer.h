#pragma once

#include "Graphics/RHI/GraphicsDevice.h"
#include <cstdint>
#include <vector>

namespace Yamen::Graphics {

    /**
     * @brief GPU buffer types
     */
    enum class BufferType {
        Vertex,
        Index,
        Constant
    };

    /**
     * @brief Buffer usage pattern
     */
    enum class BufferUsage {
        Default,    // GPU read/write, no CPU access
        Dynamic,    // GPU read, CPU write (frequent updates)
        Immutable   // GPU read only, set once at creation
    };

    /**
     * @brief GPU buffer wrapper
     * 
     * Manages vertex, index, and constant buffers.
     */
    class Buffer {
    public:
        Buffer(GraphicsDevice& device, BufferType type);
        ~Buffer();

        // Non-copyable
        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

        /**
         * @brief Create buffer with initial data
         * @param data Pointer to initial data (can be nullptr for dynamic buffers)
         * @param size Size in bytes
         * @param stride Element stride (for vertex/index buffers)
         * @param usage Buffer usage pattern
         * @return True if creation succeeded
         */
        bool Create(const void* data, uint32_t size, uint32_t stride, BufferUsage usage = BufferUsage::Default);

        /**
         * @brief Update buffer data (only for dynamic buffers)
         * @param data Pointer to new data
         * @param size Size in bytes
         */
        void Update(const void* data, uint32_t size);

        /**
         * @brief Bind buffer to pipeline
         * This is a convenience method that calls the appropriate bind function
         */
        void Bind();

        /**
         * @brief Get the D3D11 buffer
         */
        ID3D11Buffer* GetBuffer() const { return m_Buffer.Get(); }

        /**
         * @brief Get buffer properties
         */
        BufferType GetType() const { return m_Type; }
        BufferUsage GetUsage() const { return m_Usage; }
        uint32_t GetSize() const { return m_Size; }
        uint32_t GetStride() const { return m_Stride; }
        uint32_t GetCount() const { return m_Stride > 0 ? m_Size / m_Stride : 0; }

        /**
         * @brief Bind constant buffer to vertex shader
         * @param slot Constant buffer slot (0-13)
         */
        void BindToVertexShader(uint32_t slot = 0);

        /**
         * @brief Bind constant buffer to pixel shader
         * @param slot Constant buffer slot (0-13)
         */
        void BindToPixelShader(uint32_t slot = 0);

    private:
        GraphicsDevice& m_Device;
        ComPtr<ID3D11Buffer> m_Buffer;
        BufferType m_Type;
        BufferUsage m_Usage;
        uint32_t m_Size;
        uint32_t m_Stride;
    };

} // namespace Yamen::Graphics
