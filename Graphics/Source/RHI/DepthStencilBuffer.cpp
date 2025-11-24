#include "Graphics/RHI/DepthStencilBuffer.h"
#include "Core/Logging/Logger.h"

namespace Yamen::Graphics {

    DepthStencilBuffer::DepthStencilBuffer(GraphicsDevice& device)
        : m_Device(device)
        , m_Width(0)
        , m_Height(0)
        , m_HasStencil(false)
    {
    }

    DepthStencilBuffer::~DepthStencilBuffer() {
        m_DSV.Reset();
        m_Texture.Reset();
    }

    bool DepthStencilBuffer::Create(uint32_t width, uint32_t height, bool useStencil) {
        m_Width = width;
        m_Height = height;
        m_HasStencil = useStencil;

        // Choose format based on stencil requirement
        DXGI_FORMAT textureFormat = useStencil ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_D32_FLOAT;
        DXGI_FORMAT dsvFormat = textureFormat;

        // Create depth/stencil texture
        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = width;
        texDesc.Height = height;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = textureFormat;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = 0;

        HRESULT hr = m_Device.GetDevice()->CreateTexture2D(&texDesc, nullptr, &m_Texture);
        if (FAILED(hr)) {
            YAMEN_CORE_ERROR("Failed to create depth/stencil texture: 0x{:08X}", static_cast<uint32_t>(hr));
            return false;
        }

        // Create depth/stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = dsvFormat;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;

        hr = m_Device.GetDevice()->CreateDepthStencilView(m_Texture.Get(), &dsvDesc, &m_DSV);
        if (FAILED(hr)) {
            YAMEN_CORE_ERROR("Failed to create depth/stencil view: 0x{:08X}", static_cast<uint32_t>(hr));
            return false;
        }

        YAMEN_CORE_TRACE("Created depth/stencil buffer ({}x{}, stencil: {})", 
            width, height, useStencil ? "Yes" : "No");
        return true;
    }

    void DepthStencilBuffer::ClearDepth(float depth) {
        if (!m_DSV) {
            return;
        }

        m_Device.GetContext()->ClearDepthStencilView(
            m_DSV.Get(),
            D3D11_CLEAR_DEPTH,
            depth,
            0
        );
    }

    void DepthStencilBuffer::ClearStencil(uint8_t stencil) {
        if (!m_DSV || !m_HasStencil) {
            return;
        }

        m_Device.GetContext()->ClearDepthStencilView(
            m_DSV.Get(),
            D3D11_CLEAR_STENCIL,
            1.0f,
            stencil
        );
    }

    void DepthStencilBuffer::Clear(float depth, uint8_t stencil) {
        if (!m_DSV) {
            return;
        }

        UINT clearFlags = D3D11_CLEAR_DEPTH;
        if (m_HasStencil) {
            clearFlags |= D3D11_CLEAR_STENCIL;
        }

        m_Device.GetContext()->ClearDepthStencilView(
            m_DSV.Get(),
            clearFlags,
            depth,
            stencil
        );
    }

} // namespace Yamen::Graphics
