#pragma once

#include <Core/Math/Math.h>
#include <cstdint>

namespace Yamen::Graphics {

using namespace Yamen::Core;

// Forward declaration
class Material;

/// <summary>
/// Subset of a mesh with its own material
/// </summary>
struct SubMesh {
  uint32_t startIndex = 0; // Starting index in the index buffer
  uint32_t indexCount = 0; // Number of indices to draw
  uint32_t baseVertex = 0; // Base vertex location (for indexed drawing)

  Material *material = nullptr; // Material for this submesh

  // Bounding volume (AABB)
  vec3 boundsMin{-1.0f, -1.0f, -1.0f};
  vec3 boundsMax{1.0f, 1.0f, 1.0f};

  // LOD support
  float lodDistance = 0.0f; // Distance at which this LOD is active
  uint32_t lodLevel = 0;    // LOD level (0 = highest detail)
};

} // namespace Yamen::Graphics
