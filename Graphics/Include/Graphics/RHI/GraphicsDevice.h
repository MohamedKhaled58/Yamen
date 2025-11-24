#pragma once

#include <d3d11_1.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <memory>
#include <string>

namespace Yamen::Graphics {

    using Microsoft::WRL::ComPtr;

    /**
     * @brief DirectX 11 graphics device wrapper
     * 
     * Manages D3D11 device, device context, and feature level detection.
     * Provides factory methods for creating graphics resources.
     */
    class GraphicsDevice {
    public:
        GraphicsDevice();
        ~GraphicsDevice();

        // Non-copyable
        GraphicsDevice(const GraphicsDevice&) = delete;
        GraphicsDevice& operator=(const GraphicsDevice&) = delete;

        /**
         * @brief Initialize the graphics device
         * @param enableDebugLayer Enable D3D11 debug layer (development builds)
         * @return True if initialization succeeded
         */
        bool Initialize(bool enableDebugLayer = false);

        /**
         * @brief Shutdown and release all resources
         */
        void Shutdown();

        /**
         * @brief Get the D3D11 device
         */
        ID3D11Device* GetDevice() const { return m_Device.Get(); }

        /**
         * @brief Get the D3D11 device context
         */
        ID3D11DeviceContext* GetContext() const { return m_Context.Get(); }

        /**
         * @brief Get the DXGI factory
         */
        IDXGIFactory1* GetFactory() const { return m_Factory.Get(); }

        /**
         * @brief Get the feature level
         */
        D3D_FEATURE_LEVEL GetFeatureLevel() const { return m_FeatureLevel; }

        /**
         * @brief Check if device is initialized
         */
        bool IsInitialized() const { return m_Device != nullptr; }

        /**
         * @brief Get device capabilities
         */
        struct Capabilities {
            bool supportsD3D11_1;
            bool supportsMultithreading;
            bool supportsComputeShaders;
            UINT maxTexture2DSize;
            UINT maxTextureCubeSize;
        };
        const Capabilities& GetCapabilities() const { return m_Capabilities; }

    private:
        void QueryCapabilities();

        ComPtr<ID3D11Device> m_Device;
        ComPtr<ID3D11DeviceContext> m_Context;
        ComPtr<IDXGIFactory1> m_Factory;
        D3D_FEATURE_LEVEL m_FeatureLevel;
        Capabilities m_Capabilities;
        bool m_DebugLayerEnabled;
    };

} // namespace Yamen::Graphics
