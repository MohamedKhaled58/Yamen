#include "Graphics/Lighting/ShadowMap.h"
#include <Core/Logging/Logger.h>

namespace Yamen::Graphics {

    ShadowMap::ShadowMap(GraphicsDevice& device, uint32_t width, uint32_t height)
        : m_Device(device)
        , m_Width(width)
        , m_Height(height)
    {
        m_Viewport.TopLeftX = 0.0f;
        m_Viewport.TopLeftY = 0.0f;
        m_Viewport.Width = static_cast<float>(width);
        m_Viewport.Height = static_cast<float>(height);
        m_Viewport.MinDepth = 0.0f;
        m_Viewport.MaxDepth = 1.0f;
    }

    ShadowMap::~ShadowMap() {
    }

    bool ShadowMap::Initialize() {
        // Create texture
        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = m_Width;
        texDesc.Height = m_Height;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS; // Allow casting to depth and shader resource
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = 0;

        HRESULT hr = m_Device.GetDevice()->CreateTexture2D(&texDesc, nullptr, m_Texture.GetAddressOf());
        if (FAILED(hr)) {
            YAMEN_CORE_ERROR("Failed to create shadow map texture");
            return false;
        }

        // Create DSV
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;

        hr = m_Device.GetDevice()->CreateDepthStencilView(m_Texture.Get(), &dsvDesc, m_DSV.GetAddressOf());
        if (FAILED(hr)) {
            YAMEN_CORE_ERROR("Failed to create shadow map DSV");
            return false;
        }

        // Create SRV
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;

        hr = m_Device.GetDevice()->CreateShaderResourceView(m_Texture.Get(), &srvDesc, m_SRV.GetAddressOf());
        if (FAILED(hr)) {
            YAMEN_CORE_ERROR("Failed to create shadow map SRV");
            return false;
        }

        return true;
    }

    void ShadowMap::BindDSV() {
        m_Device.GetContext()->RSSetViewports(1, &m_Viewport);
        ID3D11RenderTargetView* nullRTV = nullptr;
        m_Device.GetContext()->OMSetRenderTargets(0, &nullRTV, m_DSV.Get());
    }

    void ShadowMap::BindSRV(uint32_t slot) {
        m_Device.GetContext()->PSSetShaderResources(slot, 1, m_SRV.GetAddressOf());
    }

    void ShadowMap::Clear() {
        m_Device.GetContext()->ClearDepthStencilView(m_DSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    }

} // namespace Yamen::Graphics
