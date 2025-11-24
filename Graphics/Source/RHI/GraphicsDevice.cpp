#include "Graphics/RHI/GraphicsDevice.h"
#include "Core/Logging/Logger.h"
#include <vector>

namespace Yamen::Graphics {

    GraphicsDevice::GraphicsDevice()
        : m_FeatureLevel(D3D_FEATURE_LEVEL_11_0)
        , m_DebugLayerEnabled(false)
    {
    }

    GraphicsDevice::~GraphicsDevice() {
        Shutdown();
    }

    bool GraphicsDevice::Initialize(bool enableDebugLayer) {
        YAMEN_CORE_INFO("Initializing Graphics Device...");

        m_DebugLayerEnabled = enableDebugLayer;

        // Create DXGI factory (use Factory1 for better compatibility)
        HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&m_Factory));
        if (FAILED(hr)) {
            YAMEN_CORE_ERROR("Failed to create DXGI factory: 0x{:08X}", static_cast<uint32_t>(hr));
            return false;
        }

        // Define feature levels to try (in order of preference)
        std::vector<D3D_FEATURE_LEVEL> featureLevels = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0
        };

        // Device creation flags
        UINT createDeviceFlags = 0;
#ifdef _DEBUG
        if (enableDebugLayer) {
            createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        }
#endif

        // Try to create device with each feature level
        for (D3D_FEATURE_LEVEL requestedLevel : featureLevels) {
            hr = D3D11CreateDevice(
                nullptr,                    // Use default adapter
                D3D_DRIVER_TYPE_HARDWARE,   // Hardware acceleration
                nullptr,                    // No software rasterizer
                createDeviceFlags,
                &requestedLevel,
                1,
                D3D11_SDK_VERSION,
                &m_Device,
                &m_FeatureLevel,
                &m_Context
            );

            if (SUCCEEDED(hr)) {
                break;
            }
        }

        if (FAILED(hr)) {
            YAMEN_CORE_ERROR("Failed to create D3D11 device: 0x{:08X}", static_cast<uint32_t>(hr));
            return false;
        }

        // Log feature level
        const char* featureLevelStr = "Unknown";
        switch (m_FeatureLevel) {
            case D3D_FEATURE_LEVEL_11_1: featureLevelStr = "11.1"; break;
            case D3D_FEATURE_LEVEL_11_0: featureLevelStr = "11.0"; break;
            case D3D_FEATURE_LEVEL_10_1: featureLevelStr = "10.1"; break;
            case D3D_FEATURE_LEVEL_10_0: featureLevelStr = "10.0"; break;
        }
        YAMEN_CORE_INFO("D3D11 Device created with feature level {}", featureLevelStr);

        // Query device capabilities
        QueryCapabilities();

        YAMEN_CORE_INFO("Graphics Device initialized successfully");
        return true;
    }

    void GraphicsDevice::Shutdown() {
        if (!m_Device) {
            return;
        }

        YAMEN_CORE_INFO("Shutting down Graphics Device...");

        // Release resources in reverse order
        m_Context.Reset();
        m_Device.Reset();
        m_Factory.Reset();

        YAMEN_CORE_INFO("Graphics Device shut down");
    }

    void GraphicsDevice::QueryCapabilities() {
        m_Capabilities = {};

        // Check D3D11.1 support
        ComPtr<ID3D11Device1> device1;
        HRESULT hr = m_Device.As(&device1);
        m_Capabilities.supportsD3D11_1 = SUCCEEDED(hr);

        // Check threading support
        D3D11_FEATURE_DATA_THREADING threadingCaps = {};
        hr = m_Device->CheckFeatureSupport(
            D3D11_FEATURE_THREADING,
            &threadingCaps,
            sizeof(threadingCaps)
        );
        if (SUCCEEDED(hr)) {
            m_Capabilities.supportsMultithreading = threadingCaps.DriverConcurrentCreates &&
                                                   threadingCaps.DriverCommandLists;
        }

        // Check compute shader support
        m_Capabilities.supportsComputeShaders = (m_FeatureLevel >= D3D_FEATURE_LEVEL_11_0);

        // Query texture size limits
        switch (m_FeatureLevel) {
            case D3D_FEATURE_LEVEL_11_1:
            case D3D_FEATURE_LEVEL_11_0:
                m_Capabilities.maxTexture2DSize = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
                m_Capabilities.maxTextureCubeSize = D3D11_REQ_TEXTURECUBE_DIMENSION;
                break;
            case D3D_FEATURE_LEVEL_10_1:
            case D3D_FEATURE_LEVEL_10_0:
                m_Capabilities.maxTexture2DSize = D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION;
                m_Capabilities.maxTextureCubeSize = D3D10_REQ_TEXTURECUBE_DIMENSION;
                break;
            default:
                m_Capabilities.maxTexture2DSize = 2048;
                m_Capabilities.maxTextureCubeSize = 512;
                break;
        }

        YAMEN_CORE_INFO("Device Capabilities:");
        YAMEN_CORE_INFO("  D3D11.1 Support: {}", m_Capabilities.supportsD3D11_1 ? "Yes" : "No");
        YAMEN_CORE_INFO("  Multithreading: {}", m_Capabilities.supportsMultithreading ? "Yes" : "No");
        YAMEN_CORE_INFO("  Compute Shaders: {}", m_Capabilities.supportsComputeShaders ? "Yes" : "No");
        YAMEN_CORE_INFO("  Max Texture2D Size: {}", m_Capabilities.maxTexture2DSize);
        YAMEN_CORE_INFO("  Max TextureCube Size: {}", m_Capabilities.maxTextureCubeSize);
    }

} // namespace Yamen::Graphics
