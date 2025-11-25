#pragma once

#include "Graphics/RHI/GraphicsDevice.h"

namespace Yamen::Graphics {

    /**
     * @brief Cull mode
     */
    enum class CullMode {
        None,
        Front,
        Back
    };

    /**
     * @brief Fill mode
     */
    enum class FillMode {
        Solid,
        Wireframe
    };

    /**
     * @brief Rasterizer state
     */
    class RasterizerState {
    public:
        RasterizerState(GraphicsDevice& device);
        ~RasterizerState();

        // Non-copyable
        RasterizerState(const RasterizerState&) = delete;
        RasterizerState& operator=(const RasterizerState&) = delete;

        /**
         * @brief Create rasterizer state
         */
        bool Create(
            CullMode cullMode = CullMode::Back,
            FillMode fillMode = FillMode::Solid,
            bool frontCounterClockwise = false,
            int depthBias = 0,
            float depthBiasClamp = 0.0f,
            float slopeScaledDepthBias = 0.0f,
            bool depthClipEnable = true,
            bool scissorEnable = false,
            bool multisampleEnable = false,
            bool antialiasedLineEnable = false
        );

        /**
         * @brief Bind rasterizer state
         */
        void Bind();

        /**
         * @brief Get D3D11 rasterizer state
         */
        ID3D11RasterizerState* GetRasterizerState() const { return m_RasterizerState.Get(); }

    private:
        GraphicsDevice& m_Device;
        ComPtr<ID3D11RasterizerState> m_RasterizerState;
    };

} // namespace Yamen::Graphics
