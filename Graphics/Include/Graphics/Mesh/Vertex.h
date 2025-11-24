#pragma once

#include <glm/glm.hpp>

namespace Yamen::Graphics {

    /**
     * @brief Vertex format for 3D meshes
     */
    struct Vertex3D {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoord;
        glm::vec3 tangent;

        Vertex3D() = default;
        Vertex3D(const glm::vec3& pos, const glm::vec3& norm, const glm::vec2& uv)
            : position(pos), normal(norm), texCoord(uv), tangent(0.0f) {}
    };

    /**
     * @brief Vertex format for 2D sprites
     */
    struct Vertex2D {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 texCoord;

        Vertex2D() = default;
        Vertex2D(const glm::vec3& pos, const glm::vec4& col, const glm::vec2& uv)
            : position(pos), color(col), texCoord(uv) {}
    };

} // namespace Yamen::Graphics
