#pragma once

#include "Graphics/RHI/GraphicsDevice.h"
#include <d3d11.h>
#include <wrl/client.h>

namespace Yamen::Graphics {

    class ShadowMap {
    public:
        ShadowMap(GraphicsDevice& device, uint32_t width, uint32_t height);
        ~ShadowMap();

        bool Initialize();

        // Bind as render target (for shadow pass)
        void BindDSV();

        // Bind as shader resource (for lighting pass)
        void BindSRV(uint32_t slot = 1);

        // Clear depth buffer
        void Clear();

        ID3D11ShaderResourceView* GetSRV() const { return m_SRV.Get(); }

    private:
        GraphicsDevice& m_Device;
        uint32_t m_Width;
        uint32_t m_Height;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_Texture;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_DSV;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SRV;
        D3D11_VIEWPORT m_Viewport;
    };

} // namespace Yamen::Graphics
