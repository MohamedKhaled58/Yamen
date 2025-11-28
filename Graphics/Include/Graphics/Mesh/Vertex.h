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

    /**
     * @brief Vertex format for skinned/animated meshes (C3Skin shader)
     * 
     * Used for skeletal animation with dual bone blending.
     * Supports up to 70 bones with 2 influences per vertex.
     */
    struct VertexSkinned {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 texCoord;
        glm::vec4 boneIndexWeight; // xy = bone indices (0-69), zw = weights (0-255)

        VertexSkinned() = default;
        VertexSkinned(const glm::vec3& pos, const glm::vec4& col, const glm::vec2& uv, 
                      const glm::vec4& boneData)
            : position(pos), color(col), texCoord(uv), boneIndexWeight(boneData) {}
    };

    /**
     * @brief Vertex format for particle effects (C3Ptcl shaders)
     */
    struct VertexParticle {
        glm::vec4 position; // w component can be used for particle size
        glm::vec4 color;    // Optional, used by C3Ptcl3
        glm::vec2 texCoord;

        VertexParticle() = default;
        VertexParticle(const glm::vec4& pos, const glm::vec4& col, const glm::vec2& uv)
            : position(pos), color(col), texCoord(uv) {}
        VertexParticle(const glm::vec4& pos, const glm::vec2& uv)
            : position(pos), color(1.0f), texCoord(uv) {}
    };


} // namespace Yamen::Graphics
