#pragma once

#include <Core/Math/Math.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>


namespace Yamen::Graphics {

using namespace Yamen::Core;

// Forward declarations
class GraphicsDevice;
class Texture2D;

/// <summary>
/// Region within a texture atlas
/// </summary>
struct AtlasRegion {
  std::string name;
  vec2 uvMin{0.0f, 0.0f}; // UV coordinates (0-1)
  vec2 uvMax{1.0f, 1.0f};
  vec2 size{0.0f, 0.0f};   // Pixel size
  vec2 offset{0.0f, 0.0f}; // Pixel offset in atlas
};

/// <summary>
/// Texture atlas for batched 2D rendering
/// </summary>
class TextureAtlas {
public:
  TextureAtlas() = default;
  ~TextureAtlas() = default;

  // Load atlas from image + data file
  bool Load(GraphicsDevice &device, const std::string &imagePath,
            const std::string &dataPath);

  // Create atlas from sprite images (packs them into single texture)
  bool Create(GraphicsDevice &device,
              const std::vector<std::string> &imagePaths, int maxSize = 2048);

  // Get atlas texture
  Texture2D *GetTexture() const { return m_Texture.get(); }

  // Get region by name
  const AtlasRegion *GetRegion(const std::string &name) const;
  bool HasRegion(const std::string &name) const;

  // Get all regions
  const std::unordered_map<std::string, AtlasRegion> &GetRegions() const {
    return m_Regions;
  }

  // Atlas info
  int GetWidth() const { return m_Width; }
  int GetHeight() const { return m_Height; }

private:
  std::unique_ptr<Texture2D> m_Texture;
  std::unordered_map<std::string, AtlasRegion> m_Regions;
  int m_Width = 0;
  int m_Height = 0;
};

} // namespace Yamen::Graphics
