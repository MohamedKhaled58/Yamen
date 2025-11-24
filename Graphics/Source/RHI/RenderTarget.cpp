#include "Graphics/RHI/RenderTarget.h"
#include "Core/Logging/Logger.h"

namespace Yamen::Graphics {

    RenderTarget::RenderTarget(GraphicsDevice& device)
        : m_Device(device)
        , m_Width(0)
        , m_Height(0)
    {
    }

    RenderTarget::~RenderTarget() {
        m_RTV.Reset();
    }

    bool RenderTarget::CreateFromTexture(ID3D11Texture2D* texture) {
        if (!texture) {
            YAMEN_CORE_ERROR("Cannot create render target from null texture");
            return false;
        }

        // Get texture description
        D3D11_TEXTURE2D_DESC texDesc;
        texture->GetDesc(&texDesc);
        m_Width = texDesc.Width;
        m_Height = texDesc.Height;

        // Create render target view
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = texDesc.Format;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;

        HRESULT hr = m_Device.GetDevice()->CreateRenderTargetView(
            texture,
            &rtvDesc,
            &m_RTV
        );

        if (FAILED(hr)) {
            YAMEN_CORE_ERROR("Failed to create render target view: 0x{:08X}", static_cast<uint32_t>(hr));
            return false;
        }

        YAMEN_CORE_TRACE("Created render target ({}x{})", m_Width, m_Height);
        return true;
    }

    void RenderTarget::Clear(float r, float g, float b, float a) {
        if (!m_RTV) {
            return;
        }

        float clearColor[4] = { r, g, b, a };
        m_Device.GetContext()->ClearRenderTargetView(m_RTV.Get(), clearColor);
    }

} // namespace Yamen::Graphics
