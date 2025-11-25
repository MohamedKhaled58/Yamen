#pragma once

#include "Graphics/RHI/GraphicsDevice.h"

namespace Yamen::Graphics {

    /**
     * @brief Blend mode
     */
    enum class BlendMode {
        Opaque,
        AlphaBlend,
        Additive,
        Multiply
    };

    /**
     * @brief Blend state
     */
    class BlendState {
    public:
        BlendState(GraphicsDevice& device);
        ~BlendState();

        // Non-copyable
        BlendState(const BlendState&) = delete;
        BlendState& operator=(const BlendState&) = delete;

        /**
         * @brief Create blend state
         */
        bool Create(BlendMode mode = BlendMode::Opaque, bool alphaToCoverage = false);

        /**
         * @brief Bind blend state
         */
        void Bind(const float* blendFactor = nullptr, uint32_t sampleMask = 0xffffffff);

        /**
         * @brief Get D3D11 blend state
         */
        ID3D11BlendState* GetBlendState() const { return m_BlendState.Get(); }

    private:
        GraphicsDevice& m_Device;
        ComPtr<ID3D11BlendState> m_BlendState;
    };

} // namespace Yamen::Graphics
