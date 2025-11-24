#include "Graphics/Texture/TextureLoader.h"
#include "Core/Logging/Logger.h"
#include "Core/Utils/FileSystem.h"

// stb_image for loading common formats
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Yamen::Graphics {

    std::unique_ptr<Texture2D> TextureLoader::LoadFromFile(GraphicsDevice& device, const std::string& filepath) {
        // Check file extension
        std::string ext = Core::FileSystem::GetExtension(filepath);
        
        // Convert to lowercase
        for (char& c : ext) {
            c = static_cast<char>(tolower(c));
        }

        if (ext == ".dds") {
            return LoadDDS(device, filepath);
        } else {
            return LoadSTB(device, filepath);
        }
    }

    std::unique_ptr<Texture2D> TextureLoader::LoadDDS(GraphicsDevice& device, const std::string& filepath) {
        // TODO: Implement DDS loading using DirectXTex
        // For now, return nullptr
        YAMEN_CORE_WARN("DDS loading not yet implemented");
        return nullptr;
    }

    std::unique_ptr<Texture2D> TextureLoader::LoadSTB(GraphicsDevice& device, const std::string& filepath) {
        // Load image using stb_image
        int width, height, channels;
        stbi_set_flip_vertically_on_load(false); // DirectX uses top-left origin
        
        unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 4); // Force RGBA
        if (!data) {
            YAMEN_CORE_ERROR("Failed to load texture: {}", filepath);
            return nullptr;
        }

        // Create texture
        auto texture = std::make_unique<Texture2D>(device);
        bool success = texture->Create(
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height),
            TextureFormat::R8G8B8A8_UNORM,
            data,
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

    std::unique_ptr<Texture2D> TextureLoader::CreateSolidColor(
        GraphicsDevice& device,
        uint32_t width, uint32_t height,
        uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
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
        if (!texture->Create(width, height, TextureFormat::R8G8B8A8_UNORM, pixels.data(), false)) {
            YAMEN_CORE_ERROR("Failed to create solid color texture");
            return nullptr;
        }

        return texture;
    }

} // namespace Yamen::Graphics
