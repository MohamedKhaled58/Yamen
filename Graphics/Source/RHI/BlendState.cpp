#include "Graphics/RHI/BlendState.h"
#include "Core/Logging/Logger.h"

namespace Yamen::Graphics {

    BlendState::BlendState(GraphicsDevice& device)
        : m_Device(device)
    {
    }

    BlendState::~BlendState() {
        m_BlendState.Reset();
    }

    bool BlendState::Create(BlendMode mode, bool alphaToCoverage) {
        D3D11_BLEND_DESC desc = {};
        desc.AlphaToCoverageEnable = alphaToCoverage;
        desc.IndependentBlendEnable = FALSE;

        D3D11_RENDER_TARGET_BLEND_DESC& rtDesc = desc.RenderTarget[0];

        switch (mode) {
            case BlendMode::Opaque:
                rtDesc.BlendEnable = FALSE;
                rtDesc.SrcBlend = D3D11_BLEND_ONE;
                rtDesc.DestBlend = D3D11_BLEND_ZERO;
                rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
                rtDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
                rtDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
                rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
                break;

            case BlendMode::AlphaBlend:
                rtDesc.BlendEnable = TRUE;
                rtDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
                rtDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
                rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
                rtDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
                rtDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
                rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
                break;

            case BlendMode::Additive:
                rtDesc.BlendEnable = TRUE;
                rtDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
                rtDesc.DestBlend = D3D11_BLEND_ONE;
                rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
                rtDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
                rtDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
                rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
                break;

            case BlendMode::Multiply:
                rtDesc.BlendEnable = TRUE;
                rtDesc.SrcBlend = D3D11_BLEND_DEST_COLOR;
                rtDesc.DestBlend = D3D11_BLEND_ZERO;
                rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
                rtDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
                rtDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
                rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
                break;
        }

        rtDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        HRESULT hr = m_Device.GetDevice()->CreateBlendState(&desc, &m_BlendState);
        if (FAILED(hr)) {
            YAMEN_CORE_ERROR("Failed to create blend state: 0x{:08X}", static_cast<uint32_t>(hr));
            return false;
        }

        return true;
    }

    void BlendState::Bind(const float* blendFactor, uint32_t sampleMask) {
        m_Device.GetContext()->OMSetBlendState(m_BlendState.Get(), blendFactor, sampleMask);
    }

} // namespace Yamen::Graphics
