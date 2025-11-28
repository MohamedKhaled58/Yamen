#include "Graphics/Texture/TextureLoader.h"
#include "Core/Logging/Logger.h"
#include "Core/Utils/FileSystem.h"

#include <DirectXTex.h>
#include <wrl/client.h>

// stb_image for loading common formats
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Yamen::Graphics {

std::unique_ptr<Texture2D>
TextureLoader::LoadFromFile(GraphicsDevice &device,
                            const std::string &filepath) {
  // Check file extension
  std::string ext = Core::FileSystem::GetExtension(filepath);

  // Convert to lowercase
  for (char &c : ext) {
    c = static_cast<char>(tolower(c));
  }

  if (ext == ".dds") {
    return LoadDDS(device, filepath);
  } else {
    return LoadSTB(device, filepath);
  }
}

std::unique_ptr<Texture2D> TextureLoader::LoadDDS(GraphicsDevice &device,
                                                  const std::string &filepath) {
  // Convert to wide string for DirectXTex
  std::wstring wpath(filepath.begin(), filepath.end());

  // Load DDS file using DirectXTex
  DirectX::TexMetadata metadata;
  DirectX::ScratchImage image;

  HRESULT hr = DirectX::LoadFromDDSFile(wpath.c_str(), DirectX::DDS_FLAGS_NONE,
                                        &metadata, image);
  if (FAILED(hr)) {
    YAMEN_CORE_ERROR("Failed to load DDS file: {} (HRESULT: 0x{:08X})",
                     filepath, static_cast<unsigned>(hr));
    return nullptr;
  }

  // Create D3D11 texture from loaded image
  Microsoft::WRL::ComPtr<ID3D11Resource> resource;
  hr = DirectX::CreateTexture(device.GetDevice(), image.GetImages(),
                              image.GetImageCount(), metadata,
                              resource.GetAddressOf());

  if (FAILED(hr)) {
    YAMEN_CORE_ERROR(
        "Failed to create D3D11 texture from DDS: {} (HRESULT: 0x{:08X})",
        filepath, static_cast<unsigned>(hr));
    return nullptr;
  }

  // Get ID3D11Texture2D interface
  Microsoft::WRL::ComPtr<ID3D11Texture2D> d3dTexture;
  hr = resource.As(&d3dTexture);
  if (FAILED(hr)) {
    YAMEN_CORE_ERROR(
        "Failed to get ID3D11Texture2D interface (HRESULT: 0x{:08X})",
        static_cast<unsigned>(hr));
    return nullptr;
  }

  // Create shader resource view
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
  hr = DirectX::CreateShaderResourceView(device.GetDevice(), image.GetImages(),
                                         image.GetImageCount(), metadata,
                                         srv.GetAddressOf());

  if (FAILED(hr)) {
    YAMEN_CORE_ERROR(
        "Failed to create SRV for DDS texture: {} (HRESULT: 0x{:08X})",
        filepath, static_cast<unsigned>(hr));
    return nullptr;
  }

  // Create our Texture2D wrapper
  auto texture = std::make_unique<Texture2D>(device);

  // Set the texture's internal resources
  texture->SetD3DTexture(d3dTexture.Get(), srv.Get());

  YAMEN_CORE_INFO("Loaded DDS texture: {} ({}x{}, {} mips)", filepath,
                  metadata.width, metadata.height, metadata.mipLevels);

  return texture;
}

std::unique_ptr<Texture2D> TextureLoader::LoadSTB(GraphicsDevice &device,
                                                  const std::string &filepath) {
  // Load image using stb_image
  int width, height, channels;
  stbi_set_flip_vertically_on_load(false); // DirectX uses top-left origin

  unsigned char *data =
      stbi_load(filepath.c_str(), &width, &height, &channels, 4); // Force RGBA
  if (!data) {
    YAMEN_CORE_ERROR("Failed to load texture: {}", filepath);
    return nullptr;
  }

  // Create texture
  auto texture = std::make_unique<Texture2D>(device);
  bool success = texture->Create(static_cast<uint32_t>(width),
                                 static_cast<uint32_t>(height),
                                 TextureFormat::R8G8B8A8_UNORM, data,
                                 true // Generate mipmaps
  );

  // Free stb_image data
  stbi_image_free(data);

  if (!success) {
    YAMEN_CORE_ERROR("Failed to create texture from file: {}", filepath);
    return nullptr;
  }

  YAMEN_CORE_INFO("Loaded texture: {} ({}x{})", filepath, width, height);
  return texture;
}

std::unique_ptr<Texture2D>
TextureLoader::CreateSolidColor(GraphicsDevice &device, uint32_t width,
                                uint32_t height, uint8_t r, uint8_t g,
                                uint8_t b, uint8_t a) {
  // Create pixel data
  std::vector<uint8_t> pixels(width * height * 4);
  for (uint32_t i = 0; i < width * height; ++i) {
    pixels[i * 4 + 0] = r;
    pixels[i * 4 + 1] = g;
    pixels[i * 4 + 2] = b;
    pixels[i * 4 + 3] = a;
  }

  // Create texture
  auto texture = std::make_unique<Texture2D>(device);
  if (!texture->Create(width, height, TextureFormat::R8G8B8A8_UNORM,
                       pixels.data(), false)) {
    YAMEN_CORE_ERROR("Failed to create solid color texture");
    return nullptr;
  }

  return texture;
}

} // namespace Yamen::Graphics
