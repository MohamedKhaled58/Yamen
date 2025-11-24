#pragma once

#include "Graphics/RHI/GraphicsDevice.h"
#include <cstdint>

namespace Yamen::Graphics {

    class RenderTarget;

    /**
     * @brief DXGI swap chain wrapper
     * 
     * Manages swap chain for presenting rendered frames to the window.
     * Supports VSync, fullscreen/windowed mode, and resize handling.
     */
    class SwapChain {
    public:
        SwapChain(GraphicsDevice& device);
        ~SwapChain();

        // Non-copyable
        SwapChain(const SwapChain&) = delete;
        SwapChain& operator=(const SwapChain&) = delete;

        /**
         * @brief Create swap chain for a window
         * @param windowHandle Native window handle (HWND)
         * @param width Window width
         * @param height Window height
         * @param vsync Enable vertical synchronization
         * @return True if creation succeeded
         */
        bool Create(void* windowHandle, uint32_t width, uint32_t height, bool vsync = true);

        /**
         * @brief Resize swap chain buffers
         * @param width New width
         * @param height New height
         */
        void Resize(uint32_t width, uint32_t height);

        /**
         * @brief Present the back buffer to the screen
         */
        void Present();

        /**
         * @brief Get the back buffer render target
         */
        RenderTarget* GetBackBuffer() const { return m_BackBuffer.get(); }

        /**
         * @brief Get swap chain dimensions
         */
        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }

        /**
         * @brief Get/Set VSync
         */
        bool GetVSync() const { return m_VSync; }
        void SetVSync(bool enabled) { m_VSync = enabled; }

        /**
         * @brief Get the DXGI swap chain
         */
        IDXGISwapChain1* GetSwapChain() const { return m_SwapChain.Get(); }

    private:
        void CreateBackBufferRenderTarget();
        void ReleaseBackBuffer();

        GraphicsDevice& m_Device;
        ComPtr<IDXGISwapChain1> m_SwapChain;
        std::unique_ptr<RenderTarget> m_BackBuffer;
        uint32_t m_Width;
        uint32_t m_Height;
        bool m_VSync;
    };

} // namespace Yamen::Graphics
