#pragma once

#include "Graphics/RHI/GraphicsDevice.h"
#include <string>
#include <vector>

namespace Yamen::Graphics {

    class TextureCube {
    public:
        TextureCube(GraphicsDevice& device);
        ~TextureCube();

        // Load cubemap from 6 files (Right, Left, Top, Bottom, Front, Back)
        bool Load(const std::vector<std::string>& filepaths);

        // Bind to shader slot
        void Bind(uint32_t slot = 0);

        ID3D11ShaderResourceView* GetSRV() const { return m_SRV.Get(); }

    private:
        GraphicsDevice& m_Device;
        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_Texture;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SRV;
    };

} // namespace Yamen::Graphics
