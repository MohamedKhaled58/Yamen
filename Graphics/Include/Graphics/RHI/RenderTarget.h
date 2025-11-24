#pragma once

#include "Graphics/RHI/GraphicsDevice.h"
#include <cstdint>

namespace Yamen::Graphics {

    /**
     * @brief Render target view wrapper
     * 
     * Represents a render target that can be bound to the output-merger stage.
     * Can be created from a texture or swap chain back buffer.
     */
    class RenderTarget {
    public:
        RenderTarget(GraphicsDevice& device);
        ~RenderTarget();

        // Non-copyable
        RenderTarget(const RenderTarget&) = delete;
        RenderTarget& operator=(const RenderTarget&) = delete;

        /**
         * @brief Create render target from texture
         * @param texture D3D11 texture resource
         * @return True if creation succeeded
         */
        bool CreateFromTexture(ID3D11Texture2D* texture);

        /**
         * @brief Clear the render target to a color
         * @param r Red component (0-1)
         * @param g Green component (0-1)
         * @param b Blue component (0-1)
         * @param a Alpha component (0-1)
         */
        void Clear(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);

        /**
         * @brief Get the render target view
         */
        ID3D11RenderTargetView* GetRTV() const { return m_RTV.Get(); }

        /**
         * @brief Get render target dimensions
         */
        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }

    private:
        GraphicsDevice& m_Device;
        ComPtr<ID3D11RenderTargetView> m_RTV;
        uint32_t m_Width;
        uint32_t m_Height;
    };

} // namespace Yamen::Graphics
