#pragma once

#include "Graphics/RHI/GraphicsDevice.h"

namespace Yamen::Graphics {

    /**
     * @brief Depth/Stencil state
     */
    class DepthStencilState {
    public:
        DepthStencilState(GraphicsDevice& device);
        ~DepthStencilState();

        // Non-copyable
        DepthStencilState(const DepthStencilState&) = delete;
        DepthStencilState& operator=(const DepthStencilState&) = delete;

        /**
         * @brief Create depth/stencil state
         */
        bool Create(
            bool depthEnable = true,
            bool depthWriteEnable = true,
            D3D11_COMPARISON_FUNC depthFunc = D3D11_COMPARISON_LESS,
            bool stencilEnable = false
        );

        /**
         * @brief Bind depth/stencil state
         */
        void Bind(uint32_t stencilRef = 0);

        /**
         * @brief Get D3D11 depth/stencil state
         */
        ID3D11DepthStencilState* GetDepthStencilState() const { return m_DepthStencilState.Get(); }

    private:
        GraphicsDevice& m_Device;
        ComPtr<ID3D11DepthStencilState> m_DepthStencilState;
    };

} // namespace Yamen::Graphics
