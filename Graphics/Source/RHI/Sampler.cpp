#include "Graphics/RHI/Sampler.h"
#include "Core/Logging/Logger.h"

namespace Yamen::Graphics {

    Sampler::Sampler(GraphicsDevice& device)
        : m_Device(device)
    {
    }

    Sampler::~Sampler() {
        m_SamplerState.Reset();
    }

    bool Sampler::Create(SamplerFilter filter, SamplerAddressMode addressMode, uint32_t maxAnisotropy) {
        D3D11_SAMPLER_DESC samplerDesc = {};

        // Filter mode
        switch (filter) {
            case SamplerFilter::Point:
                samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
                break;
            case SamplerFilter::Linear:
                samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
                break;
            case SamplerFilter::Anisotropic:
                samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
                break;
        }

        // Address mode
        D3D11_TEXTURE_ADDRESS_MODE d3dAddressMode;
        switch (addressMode) {
            case SamplerAddressMode::Wrap:
                d3dAddressMode = D3D11_TEXTURE_ADDRESS_WRAP;
                break;
            case SamplerAddressMode::Clamp:
                d3dAddressMode = D3D11_TEXTURE_ADDRESS_CLAMP;
                break;
            case SamplerAddressMode::Mirror:
                d3dAddressMode = D3D11_TEXTURE_ADDRESS_MIRROR;
                break;
            case SamplerAddressMode::Border:
                d3dAddressMode = D3D11_TEXTURE_ADDRESS_BORDER;
                break;
            default:
                d3dAddressMode = D3D11_TEXTURE_ADDRESS_WRAP;
                break;
        }

        samplerDesc.AddressU = d3dAddressMode;
        samplerDesc.AddressV = d3dAddressMode;
        samplerDesc.AddressW = d3dAddressMode;
        samplerDesc.MipLODBias = 0.0f;
        samplerDesc.MaxAnisotropy = maxAnisotropy;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDesc.BorderColor[0] = 0.0f;
        samplerDesc.BorderColor[1] = 0.0f;
        samplerDesc.BorderColor[2] = 0.0f;
        samplerDesc.BorderColor[3] = 0.0f;
        samplerDesc.MinLOD = 0.0f;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

        HRESULT hr = m_Device.GetDevice()->CreateSamplerState(&samplerDesc, &m_SamplerState);
        if (FAILED(hr)) {
            YAMEN_CORE_ERROR("Failed to create sampler state: 0x{:08X}", static_cast<uint32_t>(hr));
            return false;
        }

        return true;
    }

    void Sampler::Bind(uint32_t slot) {
        m_Device.GetContext()->PSSetSamplers(slot, 1, m_SamplerState.GetAddressOf());
    }

} // namespace Yamen::Graphics
