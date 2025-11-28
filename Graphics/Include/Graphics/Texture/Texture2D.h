#pragma once

#include "Graphics/RHI/GraphicsDevice.h"
#include <cstdint>
#include <string>

namespace Yamen::Graphics {

    /**
     * @brief Texture format enumeration
     */
    enum class TextureFormat {
        Unknown,
        R8G8B8A8_UNORM,
        R8G8B8A8_SRGB,
        BC1_UNORM,      // DXT1
        BC2_UNORM,      // DXT3
        BC3_UNORM,      // DXT5
        BC7_UNORM
    };

    /**
     * @brief 2D texture abstraction
     * 
     * Manages 2D textures with mipmap support.
     */
    class Texture2D {
    public:
        Texture2D(GraphicsDevice& device);
        ~Texture2D();

        // Non-copyable
        Texture2D(const Texture2D&) = delete;
        Texture2D& operator=(const Texture2D&) = delete;

        /**
         * @brief Create texture from raw data
         * @param width Texture width
         * @param height Texture height
         * @param format Texture format
         * @param data Pointer to pixel data (can be nullptr for render targets)
         * @param generateMips Auto-generate mipmaps
         * @return True if creation succeeded
         */
        bool Create(uint32_t width, uint32_t height, TextureFormat format, const void* data = nullptr, bool generateMips = false);

        /**
         * @brief Bind texture to shader slot
         * @param slot Shader resource slot (0-15)
         */
        void Bind(uint32_t slot = 0);

        /**
         * @brief Unbind texture from slot
         * @param slot Shader resource slot
         */
        void Unbind(uint32_t slot = 0);

        /**
         * @brief Get texture properties
         */
        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }
        TextureFormat GetFormat() const { return m_Format; }
        uint32_t GetMipLevels() const { return m_MipLevels; }

        /**
         * @brief Get D3D11 resources
         */
        ID3D11Texture2D* GetTexture() const { return m_Texture.Get(); }
        ID3D11ShaderResourceView* GetSRV() const { return m_SRV.Get(); }

        /**
         * @brief Set D3D11 texture from externally created resources (e.g., DirectXTex)
         * @param texture D3D11 texture resource
         * @param srv Shader resource view
         */
        void SetD3DTexture(ID3D11Texture2D* texture, ID3D11ShaderResourceView* srv);

    private:
        GraphicsDevice& m_Device;
        ComPtr<ID3D11Texture2D> m_Texture;
        ComPtr<ID3D11ShaderResourceView> m_SRV;
        uint32_t m_Width;
        uint32_t m_Height;
        TextureFormat m_Format;
        uint32_t m_MipLevels;
    };

} // namespace Yamen::Graphics
