#include "Graphics/Texture/Texture2D.h"
#include "Core/Logging/Logger.h"

namespace Yamen::Graphics {

static DXGI_FORMAT TextureFormatToDXGI(TextureFormat format) {
  switch (format) {
  case TextureFormat::R8G8B8A8_UNORM:
    return DXGI_FORMAT_R8G8B8A8_UNORM;
  case TextureFormat::R8G8B8A8_SRGB:
    return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
  case TextureFormat::BC1_UNORM:
    return DXGI_FORMAT_BC1_UNORM;
  case TextureFormat::BC2_UNORM:
    return DXGI_FORMAT_BC2_UNORM;
  case TextureFormat::BC3_UNORM:
    return DXGI_FORMAT_BC3_UNORM;
  case TextureFormat::BC7_UNORM:
    return DXGI_FORMAT_BC7_UNORM;
  default:
    return DXGI_FORMAT_UNKNOWN;
  }
}

Texture2D::Texture2D(GraphicsDevice &device)
    : m_Device(device), m_Width(0), m_Height(0),
      m_Format(TextureFormat::Unknown), m_MipLevels(1) {}

Texture2D::~Texture2D() {
  m_SRV.Reset();
  m_Texture.Reset();
}

bool Texture2D::Create(uint32_t width, uint32_t height, TextureFormat format,
                       const void *data, bool generateMips) {
  m_Width = width;
  m_Height = height;
  m_Format = format;
  m_MipLevels = generateMips ? 0 : 1; // 0 = auto-generate all mips

  DXGI_FORMAT dxgiFormat = TextureFormatToDXGI(format);
  if (dxgiFormat == DXGI_FORMAT_UNKNOWN) {
    YAMEN_CORE_ERROR("Unsupported texture format");
    return false;
  }

  // Calculate mip levels if auto-generating
  if (generateMips) {
    m_MipLevels = 1;
    uint32_t w = width;
    uint32_t h = height;
    while (w > 1 || h > 1) {
      w = w > 1 ? w / 2 : 1;
      h = h > 1 ? h / 2 : 1;
      m_MipLevels++;
    }
  }

  // Create texture
  D3D11_TEXTURE2D_DESC texDesc = {};
  texDesc.Width = width;
  texDesc.Height = height;
  texDesc.MipLevels = m_MipLevels;
  texDesc.ArraySize = 1;
  texDesc.Format = dxgiFormat;
  texDesc.SampleDesc.Count = 1;
  texDesc.SampleDesc.Quality = 0;
  texDesc.Usage = D3D11_USAGE_DEFAULT;
  texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  texDesc.CPUAccessFlags = 0;
  texDesc.MiscFlags = generateMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

  D3D11_SUBRESOURCE_DATA initData = {};
  initData.pSysMem = data;
  initData.SysMemPitch = width * 4; // Assuming RGBA8

  HRESULT hr = m_Device.GetDevice()->CreateTexture2D(
      &texDesc, data ? &initData : nullptr, &m_Texture);

  if (FAILED(hr)) {
    YAMEN_CORE_ERROR("Failed to create texture: 0x{:08X}",
                     static_cast<uint32_t>(hr));
    return false;
  }

  // Create shader resource view
  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
  srvDesc.Format = dxgiFormat;
  srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MostDetailedMip = 0;
  srvDesc.Texture2D.MipLevels = m_MipLevels;

  hr = m_Device.GetDevice()->CreateShaderResourceView(m_Texture.Get(), &srvDesc,
                                                      &m_SRV);

  if (FAILED(hr)) {
    YAMEN_CORE_ERROR("Failed to create shader resource view: 0x{:08X}",
                     static_cast<uint32_t>(hr));
    return false;
  }

  // Generate mipmaps if requested and data was provided
  if (generateMips && data) {
    m_Device.GetContext()->GenerateMips(m_SRV.Get());
  }

  YAMEN_CORE_TRACE("Created texture ({}x{}, {} mips)", width, height,
                   m_MipLevels);
  return true;
}

void Texture2D::Bind(uint32_t slot) {
  if (!m_SRV) {
    return;
  }

  m_Device.GetContext()->PSSetShaderResources(slot, 1, m_SRV.GetAddressOf());
}

void Texture2D::SetD3DTexture(ID3D11Texture2D *texture,
                              ID3D11ShaderResourceView *srv) {
  // Store the externally created resources
  m_Texture = texture;
  m_SRV = srv;

  // Get texture description to update internal state
  if (texture) {
    D3D11_TEXTURE2D_DESC desc;
    texture->GetDesc(&desc);
    m_Width = desc.Width;
    m_Height = desc.Height;
    m_MipLevels = desc.MipLevels;
    // Format needs to be set based on DXGI format - for now set to unknown
    m_Format = TextureFormat::Unknown;
  }
}

void Texture2D::Unbind(uint32_t slot) {
  ID3D11ShaderResourceView *nullSRV = nullptr;
  m_Device.GetContext()->PSSetShaderResources(slot, 1, &nullSRV);
}

} // namespace Yamen::Graphics
