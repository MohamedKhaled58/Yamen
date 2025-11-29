#pragma once

#include "Graphics/Mesh/Vertex.h"
#include <vector>

namespace Yamen::Graphics {

/**
 * @brief Procedural mesh generation
 */
class MeshBuilder {
public:
  /**
   * @brief Create cube mesh
   */
  static void CreateCube(std::vector<Vertex3D> &vertices,
                         std::vector<uint32_t> &indices, float size = 1.0f);

  /**
   * @brief Create sphere mesh
   */
  static void CreateSphere(std::vector<Vertex3D> &vertices,
                           std::vector<uint32_t> &indices, float radius = 1.0f,
                           uint32_t segments = 32, uint32_t rings = 16);

  /**
   * @brief Create plane mesh
   */
  static void CreatePlane(std::vector<Vertex3D> &vertices,
                          std::vector<uint32_t> &indices, float width = 1.0f,
                          float height = 1.0f, uint32_t subdivisionsX = 1,
                          uint32_t subdivisionsY = 1);

  /**
   * @brief Calculate tangents for mesh
   */
  static void CalculateTangents(std::vector<Vertex3D> &vertices,
                                const std::vector<uint32_t> &indices);
};

} // namespace Yamen::Graphics
