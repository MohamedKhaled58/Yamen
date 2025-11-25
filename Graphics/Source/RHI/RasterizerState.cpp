#include "Graphics/RHI/RasterizerState.h"
#include "Core/Logging/Logger.h"

namespace Yamen::Graphics {

    RasterizerState::RasterizerState(GraphicsDevice& device)
        : m_Device(device)
    {
    }

    RasterizerState::~RasterizerState() {
        m_RasterizerState.Reset();
    }

    bool RasterizerState::Create(
        CullMode cullMode,
        FillMode fillMode,
        bool frontCounterClockwise,
        int depthBias,
        float depthBiasClamp,
        float slopeScaledDepthBias,
        bool depthClipEnable,
        bool scissorEnable,
        bool multisampleEnable,
        bool antialiasedLineEnable)
    {
        D3D11_RASTERIZER_DESC desc = {};

        // Cull mode
        switch (cullMode) {
            case CullMode::None:
                desc.CullMode = D3D11_CULL_NONE;
                break;
            case CullMode::Front:
                desc.CullMode = D3D11_CULL_FRONT;
                break;
            case CullMode::Back:
                desc.CullMode = D3D11_CULL_BACK;
                break;
        }

        // Fill mode
        switch (fillMode) {
            case FillMode::Solid:
                desc.FillMode = D3D11_FILL_SOLID;
                break;
            case FillMode::Wireframe:
                desc.FillMode = D3D11_FILL_WIREFRAME;
                break;
        }

        desc.FrontCounterClockwise = frontCounterClockwise;
        desc.DepthBias = depthBias;
        desc.DepthBiasClamp = depthBiasClamp;
        desc.SlopeScaledDepthBias = slopeScaledDepthBias;
        desc.DepthClipEnable = depthClipEnable;
        desc.ScissorEnable = scissorEnable;
        desc.MultisampleEnable = multisampleEnable;
        desc.AntialiasedLineEnable = antialiasedLineEnable;

        HRESULT hr = m_Device.GetDevice()->CreateRasterizerState(&desc, &m_RasterizerState);
        if (FAILED(hr)) {
            YAMEN_CORE_ERROR("Failed to create rasterizer state: 0x{:08X}", static_cast<uint32_t>(hr));
            return false;
        }

        return true;
    }

    void RasterizerState::Bind() {
        m_Device.GetContext()->RSSetState(m_RasterizerState.Get());
    }

} // namespace Yamen::Graphics
