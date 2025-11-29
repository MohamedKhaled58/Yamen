#pragma once

#include <Core/Math/Math.h>
#include <cstdint>
#include <string>
#include <vector>

namespace Yamen::Assets {

using namespace Yamen::Core;

/**
 * @brief C3 Keyframe format types
 */
enum class C3KeyframeFormat {
  Legacy, // Full matrices per bone per frame
  KKEY,   // Full 4x4 matrices per keyframe
  XKEY,   // Compressed 3x4 matrices (12 floats)
  ZKEY    // Quaternion + translation (most compressed)
};

/**
 * @brief Compressed matrix format (XKEY)
 * Stores 3x4 transformation matrix (removes 4th column which is always 0,0,0,1)
 */
struct TidyMatrix {
  float _11, _12, _13;
  float _21, _22, _23;
  float _31, _32, _33;
  float _41, _42, _43; // Translation

  mat4 ToMat4() const {
    // Build matrix in GLM column-major format, then transpose to row-major
    // Reference code uses row-major: mat._41 = translation.x
    // DirectXMath is Row-Major.
    // If the input data is row-major (which it seems to be based on _41 being
    // translation x), then we can construct it directly. _11 _12 _13 0 _21 _22
    // _23 0 _31 _32 _33 0 _41 _42 _43 1

    return mat4(_11, _12, _13, 0.0f, _21, _22, _23, 0.0f, _31, _32, _33, 0.0f,
                _41, _42, _43, 1.0f);
  }
};

/**
 * @brief Quaternion format (ZKEY)
 * Maximum compression using quaternion for rotation
 */
struct DivInfo {
  quat quaternion;  // Rotation
  vec3 translation; // Translation

  mat4 ToMat4() const {
    // Create rotation matrix from quaternion
    mat4 rot = Math::ToMat4(quaternion);
    // Apply translation
    return Math::Translate(rot, translation);
  }
};

/**
 * @brief Single keyframe data
 */
struct C3Keyframe {
  uint32_t framePosition = 0;     // Frame index
  std::vector<mat4> boneMatrices; // Bone transformations
};

/**
 * @brief Animation/Motion data
 */
struct C3Motion {
  uint32_t boneCount;      // Number of bones
  uint32_t frameCount;     // Total frames
  uint32_t keyframeCount;  // Number of keyframes
  C3KeyframeFormat format; // Keyframe format

  std::vector<C3Keyframe> keyframes; // Keyframe data
  std::vector<mat4> currentBones;    // Current bone matrices (interpolated)

  // Morph target data
  uint32_t morphCount;             // Number of morph targets
  std::vector<float> morphWeights; // [morphCount * frameCount]

  int currentFrame; // Current playback frame

  C3Motion()
      : boneCount(0), frameCount(0), keyframeCount(0),
        format(C3KeyframeFormat::Legacy), morphCount(0), currentFrame(0) {}
};

/**
 * @brief PHY vertex format
 */
struct PhyVertex {
  vec3 position;
  vec3 normal;
  vec2 texCoord;

  // Skinning data (up to 4 bone influences)
  uint8_t boneIndices[4];
  float boneWeights[4];

  PhyVertex() {
    position = vec3(0.0f);
    normal = vec3(0.0f, 1.0f, 0.0f);
    texCoord = vec2(0.0f);
    boneIndices[0] = boneIndices[1] = boneIndices[2] = boneIndices[3] = 0;
    boneWeights[0] = 1.0f;
    boneWeights[1] = boneWeights[2] = boneWeights[3] = 0.0f;
  }
};

/**
 * @brief PHY mesh data
 */
struct C3Phy {
  uint32_t blendCount; // Bones per vertex (usually 2 or 4)

  // Vertex data
  uint32_t normalVertexCount;      // Opaque vertices
  uint32_t alphaVertexCount;       // Transparent vertices
  std::vector<PhyVertex> vertices; // All vertices

  // Index data
  uint32_t normalTriCount;       // Opaque triangles
  uint32_t alphaTriCount;        // Transparent triangles
  std::vector<uint16_t> indices; // All indices

  // Texture data
  std::string textureName;  // Primary texture
  std::string textureName2; // Secondary texture
  vec2 uvAnimStep;          // UV animation offset per frame
  uint32_t textureRows;     // Texture atlas rows

  // Material
  vec4 color; // RGBA color

  // Animation
  C3Motion *motion; // Associated animation (can be null)

  // Rendering flags
  bool shouldDraw;

  // Inverse Bind Pose Matrices (calculated from Frame 0)
  std::vector<mat4> invBindMatrices;

  C3Phy()
      : blendCount(0), normalVertexCount(0), alphaVertexCount(0),
        normalTriCount(0), alphaTriCount(0), uvAnimStep(0.0f), textureRows(1),
        color(1.0f), motion(nullptr), shouldDraw(true) {}

  ~C3Phy() {
    if (motion) {
      delete motion;
      motion = nullptr;
    }
  }
};

/**
 * @brief C3 PHY file loader
 *
 * Loads .c3 files containing skeletal meshes with animation data.
 * Supports all keyframe formats: KKEY, XKEY, ZKEY, and legacy.
 */
class C3PhyLoader {
public:
  /**
   * @brief Load PHY file from disk
   * @param filepath Path to .c3 file
   * @param outPhy Output PHY data
   * @return True if successful
   */
  static bool Load(const std::string &filepath, C3Phy &outPhy);

  /**
   * @brief Load PHY from memory buffer
   * @param data Binary data
   * @param size Data size in bytes
   * @param outPhy Output PHY data
   * @return True if successful
   */
  static bool LoadFromMemory(const uint8_t *data, size_t size, C3Phy &outPhy);
  static uint32_t GetBoneIndexForMesh(const std::string &name);

  /**
   * @brief Interpolate bone matrices for a specific frame
   * @param motion Motion data
   * @param frame Target frame (can be fractional for smooth interpolation)
   * @param outMatrices Output bone matrices
   */
  static void InterpolateBones(const C3Motion &motion, float frame,
                               std::vector<mat4> &outMatrices);

private:
  // Internal parsing functions — NOW SAFE with chunkEnd bounds
  static bool ParseMotionChunk(const uint8_t *data, size_t &offset,
                               size_t chunkEnd, C3Motion &motion);
  static bool ParsePhysicsChunk(const uint8_t *data, size_t &offset,
                                size_t chunkEnd, C3Phy &phy,
                                bool isPhy4 = false);
  static bool ParseKeyframesKKEY(const uint8_t *data, size_t &offset,
                                 size_t chunkEnd, C3Motion &motion);
  static bool ParseKeyframesXKEY(const uint8_t *data, size_t &offset,
                                 size_t chunkEnd, C3Motion &motion);
  static bool ParseKeyframesZKEY(const uint8_t *data, size_t &offset,
                                 size_t chunkEnd, C3Motion &motion);
  static bool ParseKeyframesLegacy(const uint8_t *data, size_t &offset,
                                   size_t chunkEnd, C3Motion &motion);

  // Helper functions
  template <typename T>
  static bool Read(const uint8_t *data, size_t &offset, size_t dataSize,
                   T &out) {
    if (offset + sizeof(T) > dataSize)
      return false;
    memcpy(&out, data + offset, sizeof(T));
    offset += sizeof(T);
    return true;
  }
  static bool ReadString(const uint8_t *data, size_t &offset, size_t dataSize,
                         std::string &out);
  static bool ReadChunkHeader(const uint8_t *data, size_t &offset,
                              size_t dataSize, char chunkID[4],
                              uint32_t &chunkSize);
};

} // namespace Yamen::Assets
