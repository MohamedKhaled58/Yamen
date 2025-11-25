#include "Graphics/Texture/TextureAtlas.h"
#include "Graphics/Texture/Texture2D.h"
#include "Graphics/Texture/TextureLoader.h"
#include "Graphics/RHI/GraphicsDevice.h"
#include "Core/Logging/Logger.h"
#include <fstream>

namespace Yamen::Graphics {

    bool TextureAtlas::Load(GraphicsDevice& device, const std::string& imagePath, const std::string& dataPath) {
        // Load atlas texture
        m_Texture = TextureLoader::LoadFromFile(device, imagePath);
        if (!m_Texture) {
            YAMEN_CORE_ERROR("Failed to load texture atlas image: {}", imagePath);
            return false;
        }

        m_Width = m_Texture->GetWidth();
        m_Height = m_Texture->GetHeight();

        // Load region data (simple format: name x y width height per line)
        std::ifstream file(dataPath);
        if (!file.is_open()) {
            YAMEN_CORE_ERROR("Failed to open texture atlas data: {}", dataPath);
            return false;
        }

        std::string name;
        float x, y, width, height;
        while (file >> name >> x >> y >> width >> height) {
            AtlasRegion region;
            region.name = name;
            region.offset = glm::vec2(x, y);
            region.size = glm::vec2(width, height);
            region.uvMin = glm::vec2(x / m_Width, y / m_Height);
            region.uvMax = glm::vec2((x + width) / m_Width, (y + height) / m_Height);

            m_Regions[name] = region;
        }

        YAMEN_CORE_INFO("Loaded texture atlas: {} regions from {}", m_Regions.size(), imagePath);
        return true;
    }

    bool TextureAtlas::Create(GraphicsDevice& device, const std::vector<std::string>& imagePaths, int maxSize) {
        // TODO: Implement texture packing algorithm
        // For now, this is a placeholder that would:
        // 1. Load all source images
        // 2. Use a bin-packing algorithm to arrange them
        // 3. Create a single texture with all images
        // 4. Generate UV coordinates for each region
        
        YAMEN_CORE_WARN("TextureAtlas::Create not fully implemented. Use Load() with pre-packed atlas.");
        (void)device;
        (void)imagePaths;
        (void)maxSize;
        return false;
    }

    const AtlasRegion* TextureAtlas::GetRegion(const std::string& name) const {
        auto it = m_Regions.find(name);
        return (it != m_Regions.end()) ? &it->second : nullptr;
    }

    bool TextureAtlas::HasRegion(const std::string& name) const {
        return m_Regions.find(name) != m_Regions.end();
    }

} // namespace Yamen::Graphics
