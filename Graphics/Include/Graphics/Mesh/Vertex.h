#pragma once

#include <Core/Math/Math.h>

namespace Yamen::Graphics {

using namespace Yamen::Core;

/**
 * @brief Vertex format for 3D meshes
 */
struct Vertex3D {
  vec3 position;
  vec3 normal;
  vec2 texCoord;
  vec3 tangent;

  Vertex3D() = default;
  Vertex3D(const vec3 &pos, const vec3 &norm, const vec2 &uv)
      : position(pos), normal(norm), texCoord(uv), tangent(0.0f) {}
};

/**
 * @brief Vertex format for 2D sprites
 */
struct Vertex2D {
  vec3 position;
  vec4 color;
  vec2 texCoord;

  Vertex2D() = default;
  Vertex2D(const vec3 &pos, const vec4 &col, const vec2 &uv)
      : position(pos), color(col), texCoord(uv) {}
};

/**
 * @brief Vertex format for skinned/animated meshes (C3Skin shader)
 *
 * Used for skeletal animation with dual bone blending.
 * Supports up to 70 bones with 2 influences per vertex.
 */
struct VertexSkinned {
  vec3 position;
  vec4 color;
  vec2 texCoord;
  vec4 boneIndexWeight; // xy = bone indices (0-69), zw = weights (0-255)

  VertexSkinned() = default;
  VertexSkinned(const vec3 &pos, const vec4 &col, const vec2 &uv,
                const vec4 &boneData)
      : position(pos), color(col), texCoord(uv), boneIndexWeight(boneData) {}
};

/**
 * @brief Vertex format for particle effects (C3Ptcl shaders)
 */
struct VertexParticle {
  vec4 position; // w component can be used for particle size
  vec4 color;    // Optional, used by C3Ptcl3
  vec2 texCoord;

  VertexParticle() = default;
  VertexParticle(const vec4 &pos, const vec4 &col, const vec2 &uv)
      : position(pos), color(col), texCoord(uv) {}
  VertexParticle(const vec4 &pos, const vec2 &uv)
      : position(pos), color(1.0f), texCoord(uv) {}
};

} // namespace Yamen::Graphics
