#include "Graphics/RHI/DepthStencilState.h"
#include <Core/Logging/Logger.h>

namespace Yamen::Graphics {

    DepthStencilState::DepthStencilState(GraphicsDevice& device)
        : m_Device(device)
    {
    }

    DepthStencilState::~DepthStencilState() {
        m_DepthStencilState.Reset();
    }

    bool DepthStencilState::Create(
        bool depthEnable,
        bool depthWriteEnable,
        D3D11_COMPARISON_FUNC depthFunc,
        bool stencilEnable)
    {
        D3D11_DEPTH_STENCIL_DESC desc = {};
        desc.DepthEnable = depthEnable;
        desc.DepthWriteMask = depthWriteEnable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
        desc.DepthFunc = depthFunc;
        desc.StencilEnable = stencilEnable;
        desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
        desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

        // Front-facing stencil operations
        desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

        // Back-facing stencil operations
        desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

        HRESULT hr = m_Device.GetDevice()->CreateDepthStencilState(&desc, &m_DepthStencilState);
        if (FAILED(hr)) {
            YAMEN_CORE_ERROR("Failed to create depth/stencil state: 0x{:08X}", static_cast<uint32_t>(hr));
            return false;
        }

        return true;
    }

    void DepthStencilState::Bind(uint32_t stencilRef) {
        m_Device.GetContext()->OMSetDepthStencilState(m_DepthStencilState.Get(), stencilRef);
    }

} // namespace Yamen::Graphics
