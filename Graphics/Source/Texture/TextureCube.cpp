#include "Graphics/Texture/TextureCube.h"
#include "Core/Logging/Logger.h"
#include <DirectXTex.h>

namespace Yamen::Graphics {

    TextureCube::TextureCube(GraphicsDevice& device)
        : m_Device(device)
    {
    }

    TextureCube::~TextureCube() {
    }

    bool TextureCube::Load(const std::vector<std::string>& filepaths) {
        if (filepaths.size() != 6) {
            YAMEN_CORE_ERROR("TextureCube requires exactly 6 file paths");
            return false;
        }

        // Load the 6 textures
        std::vector<DirectX::ScratchImage> images(6);
        DirectX::TexMetadata metadata;

        for (size_t i = 0; i < 6; ++i) {
            std::wstring wpath(filepaths[i].begin(), filepaths[i].end());
            HRESULT hr = DirectX::LoadFromDDSFile(wpath.c_str(), DirectX::DDS_FLAGS_NONE, &metadata, images[i]);
            
            if (FAILED(hr)) {
                // Try TGA/PNG/JPG if DDS fails
                hr = DirectX::LoadFromWICFile(wpath.c_str(), DirectX::WIC_FLAGS_NONE, &metadata, images[i]);
                if (FAILED(hr)) {
                    hr = DirectX::LoadFromTGAFile(wpath.c_str(), &metadata, images[i]);
                }
            }

            if (FAILED(hr)) {
                YAMEN_CORE_ERROR("Failed to load texture for cubemap face {0}: {1}", i, filepaths[i]);
                return false;
            }
        }

        // Create the texture array (Cubemap)
        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = static_cast<UINT>(metadata.width);
        texDesc.Height = static_cast<UINT>(metadata.height);
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 6;
        texDesc.Format = metadata.format;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

        D3D11_SUBRESOURCE_DATA data[6];
        for (size_t i = 0; i < 6; ++i) {
            data[i].pSysMem = images[i].GetPixels();
            data[i].SysMemPitch = static_cast<UINT>(images[i].GetImages()[0].rowPitch);
            data[i].SysMemSlicePitch = static_cast<UINT>(images[i].GetImages()[0].slicePitch);
        }

        HRESULT hr = m_Device.GetDevice()->CreateTexture2D(&texDesc, data, m_Texture.GetAddressOf());
        if (FAILED(hr)) {
            YAMEN_CORE_ERROR("Failed to create cubemap texture");
            return false;
        }

        // Create SRV
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = texDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.TextureCube.MostDetailedMip = 0;
        srvDesc.TextureCube.MipLevels = texDesc.MipLevels;

        hr = m_Device.GetDevice()->CreateShaderResourceView(m_Texture.Get(), &srvDesc, m_SRV.GetAddressOf());
        if (FAILED(hr)) {
            YAMEN_CORE_ERROR("Failed to create cubemap SRV");
            return false;
        }

        return true;
    }

    void TextureCube::Bind(uint32_t slot) {
        if (m_SRV) {
            m_Device.GetContext()->PSSetShaderResources(slot, 1, m_SRV.GetAddressOf());
        }
    }

} // namespace Yamen::Graphics
