#pragma once

#include "Graphics/RHI/GraphicsDevice.h"
#include <cstdint>

namespace Yamen::Graphics {

    /**
     * @brief Depth/stencil buffer wrapper
     * 
     * Manages depth/stencil texture and view for 3D rendering.
     */
    class DepthStencilBuffer {
    public:
        DepthStencilBuffer(GraphicsDevice& device);
        ~DepthStencilBuffer();

        // Non-copyable
        DepthStencilBuffer(const DepthStencilBuffer&) = delete;
        DepthStencilBuffer& operator=(const DepthStencilBuffer&) = delete;

        /**
         * @brief Create depth/stencil buffer
         * @param width Buffer width
         * @param height Buffer height
         * @param useStencil Enable stencil buffer (8-bit)
         * @return True if creation succeeded
         */
        bool Create(uint32_t width, uint32_t height, bool useStencil = true);

        /**
         * @brief Clear depth buffer
         * @param depth Depth value (0-1, default 1.0 = far plane)
         */
        void ClearDepth(float depth = 1.0f);

        /**
         * @brief Clear stencil buffer
         * @param stencil Stencil value (0-255)
         */
        void ClearStencil(uint8_t stencil = 0);

        /**
         * @brief Clear both depth and stencil
         */
        void Clear(float depth = 1.0f, uint8_t stencil = 0);

        /**
         * @brief Get the depth/stencil view
         */
        ID3D11DepthStencilView* GetDSV() const { return m_DSV.Get(); }

        /**
         * @brief Get buffer dimensions
         */
        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }

    private:
        GraphicsDevice& m_Device;
        ComPtr<ID3D11Texture2D> m_Texture;
        ComPtr<ID3D11DepthStencilView> m_DSV;
        uint32_t m_Width;
        uint32_t m_Height;
        bool m_HasStencil;
    };

} // namespace Yamen::Graphics
