#include "Graphics/RHI/SwapChain.h"
#include "Graphics/RHI/RenderTarget.h"
#include "Core/Logging/Logger.h"

namespace Yamen::Graphics {

    SwapChain::SwapChain(GraphicsDevice& device)
        : m_Device(device)
        , m_Width(0)
        , m_Height(0)
        , m_VSync(true)
    {
    }

    SwapChain::~SwapChain() {
        ReleaseBackBuffer();
        m_SwapChain.Reset();
    }

    bool SwapChain::Create(void* windowHandle, uint32_t width, uint32_t height, bool vsync) {
        if (!windowHandle) {
            YAMEN_CORE_ERROR("Invalid window handle");
            return false;
        }

        m_Width = width;
        m_Height = height;
        m_VSync = vsync;

        // Describe swap chain
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = width;
        swapChainDesc.Height = height;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.Stereo = FALSE;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2; // Double buffering
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        swapChainDesc.Flags = 0;

        // Try to get IDXGIFactory2 for modern swap chain creation
        ComPtr<IDXGIFactory2> factory2;
        HRESULT hr = m_Device.GetFactory()->QueryInterface(IID_PPV_ARGS(&factory2));
        
        if (SUCCEEDED(hr) && factory2) {
            // Use modern swap chain creation
            hr = factory2->CreateSwapChainForHwnd(
                m_Device.GetDevice(),
                static_cast<HWND>(windowHandle),
                &swapChainDesc,
                nullptr,
                nullptr,
                &m_SwapChain
            );
        } else {
            // Fallback to legacy swap chain creation
            DXGI_SWAP_CHAIN_DESC legacyDesc = {};
            legacyDesc.BufferDesc.Width = width;
            legacyDesc.BufferDesc.Height = height;
            legacyDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            legacyDesc.BufferDesc.RefreshRate.Numerator = 60;
            legacyDesc.BufferDesc.RefreshRate.Denominator = 1;
            legacyDesc.SampleDesc.Count = 1;
            legacyDesc.SampleDesc.Quality = 0;
            legacyDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            legacyDesc.BufferCount = 2;
            legacyDesc.OutputWindow = static_cast<HWND>(windowHandle);
            legacyDesc.Windowed = TRUE;
            legacyDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
            legacyDesc.Flags = 0;

            ComPtr<IDXGISwapChain> legacySwapChain;
            hr = m_Device.GetFactory()->CreateSwapChain(
                m_Device.GetDevice(),
                &legacyDesc,
                &legacySwapChain
            );

            if (SUCCEEDED(hr)) {
                legacySwapChain.As(&m_SwapChain);
            }
        }

        if (FAILED(hr)) {
            YAMEN_CORE_ERROR("Failed to create swap chain: 0x{:08X}", static_cast<uint32_t>(hr));
            return false;
        }

        // Disable Alt+Enter fullscreen toggle (we'll handle it manually)
        if (factory2) {
            factory2->MakeWindowAssociation(
                static_cast<HWND>(windowHandle),
                DXGI_MWA_NO_ALT_ENTER
            );
        }

        // Create back buffer render target
        CreateBackBufferRenderTarget();

        YAMEN_CORE_INFO("Swap chain created ({}x{}, VSync: {})", width, height, vsync ? "On" : "Off");
        return true;
    }

    void SwapChain::Resize(uint32_t width, uint32_t height) {
        if (width == m_Width && height == m_Height) {
            return;
        }

        m_Width = width;
        m_Height = height;

        // Release back buffer before resizing
        ReleaseBackBuffer();

        // Resize swap chain buffers
        HRESULT hr = m_SwapChain->ResizeBuffers(
            0, // Keep buffer count
            width,
            height,
            DXGI_FORMAT_UNKNOWN, // Keep format
            0
        );

        if (FAILED(hr)) {
            YAMEN_CORE_ERROR("Failed to resize swap chain: 0x{:08X}", static_cast<uint32_t>(hr));
            return;
        }

        // Recreate back buffer render target
        CreateBackBufferRenderTarget();

        YAMEN_CORE_INFO("Swap chain resized to {}x{}", width, height);
    }

    void SwapChain::Present() {
        if (!m_SwapChain) {
            return;
        }

        // Present with VSync (1) or without (0)
        UINT syncInterval = m_VSync ? 1 : 0;
        m_SwapChain->Present(syncInterval, 0);
    }

    void SwapChain::CreateBackBufferRenderTarget() {
        // Get back buffer texture
        ComPtr<ID3D11Texture2D> backBuffer;
        HRESULT hr = m_SwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
        if (FAILED(hr)) {
            YAMEN_CORE_ERROR("Failed to get swap chain back buffer: 0x{:08X}", static_cast<uint32_t>(hr));
            return;
        }

        // Create render target
        m_BackBuffer = std::make_unique<RenderTarget>(m_Device);
        if (!m_BackBuffer->CreateFromTexture(backBuffer.Get())) {
            YAMEN_CORE_ERROR("Failed to create back buffer render target");
            m_BackBuffer.reset();
        }
    }

    void SwapChain::ReleaseBackBuffer() {
        m_BackBuffer.reset();
    }

} // namespace Yamen::Graphics
