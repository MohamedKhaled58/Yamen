#pragma once

#include "Graphics/RHI/GraphicsDevice.h"

namespace Yamen::Graphics {

    /**
     * @brief Sampler filter mode
     */
    enum class SamplerFilter {
        Point,
        Linear,
        Anisotropic
    };

    /**
     * @brief Sampler address mode
     */
    enum class SamplerAddressMode {
        Wrap,
        Clamp,
        Mirror,
        Border
    };

    /**
     * @brief Texture sampler state
     */
    class Sampler {
    public:
        Sampler(GraphicsDevice& device);
        ~Sampler();

        // Non-copyable
        Sampler(const Sampler&) = delete;
        Sampler& operator=(const Sampler&) = delete;

        /**
         * @brief Create sampler state
         */
        bool Create(
            SamplerFilter filter = SamplerFilter::Linear,
            SamplerAddressMode addressMode = SamplerAddressMode::Wrap,
            uint32_t maxAnisotropy = 16
        );

        /**
         * @brief Bind sampler to shader slot
         */
        void Bind(uint32_t slot = 0);

        /**
         * @brief Get D3D11 sampler state
         */
        ID3D11SamplerState* GetSamplerState() const { return m_SamplerState.Get(); }

    private:
        GraphicsDevice& m_Device;
        ComPtr<ID3D11SamplerState> m_SamplerState;
    };

} // namespace Yamen::Graphics
