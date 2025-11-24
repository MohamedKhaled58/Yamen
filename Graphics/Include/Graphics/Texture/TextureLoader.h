#pragma once

#include "Graphics/Texture/Texture2D.h"
#include <string>
#include <memory>

namespace Yamen::Graphics {

    /**
     * @brief Texture loader for DDS and TGA files
     * 
     * Loads textures from disk using DirectXTex and stb_image.
     */
    class TextureLoader {
    public:
        /**
         * @brief Load texture from file
         * @param device Graphics device
         * @param filepath Path to texture file
         * @return Loaded texture or nullptr on failure
         */
        static std::unique_ptr<Texture2D> LoadFromFile(GraphicsDevice& device, const std::string& filepath);

        /**
         * @brief Load DDS texture
         * @param device Graphics device
         * @param filepath Path to DDS file
         * @return Loaded texture or nullptr on failure
         */
        static std::unique_ptr<Texture2D> LoadDDS(GraphicsDevice& device, const std::string& filepath);

        /**
         * @brief Load TGA/PNG/JPG texture using stb_image
         * @param device Graphics device
         * @param filepath Path to image file
         * @return Loaded texture or nullptr on failure
         */
        static std::unique_ptr<Texture2D> LoadSTB(GraphicsDevice& device, const std::string& filepath);

        /**
         * @brief Create a solid color texture
         * @param device Graphics device
         * @param width Texture width
         * @param height Texture height
         * @param r Red component (0-255)
         * @param g Green component (0-255)
         * @param b Blue component (0-255)
         * @param a Alpha component (0-255)
         * @return Created texture
         */
        static std::unique_ptr<Texture2D> CreateSolidColor(
            GraphicsDevice& device,
            uint32_t width, uint32_t height,
            uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255
        );
    };

} // namespace Yamen::Graphics
