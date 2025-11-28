#include "AssetsC3/C3PhyLoader.h"
#include "Core/Logging/Logger.h"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Yamen {
namespace Assets {

// ===================================================================
// SAFE READ HELPER (Critical!)
// ===================================================================
template <typename T>
static bool Read(const uint8_t *data, size_t &offset, size_t dataSize, T &out) {
  if (offset + sizeof(T) > dataSize) {
    return false;
  }
  memcpy(&out, data + offset, sizeof(T));
  offset += sizeof(T);
  return true;
}

// Helper for strings
static bool ReadString(const uint8_t *data, size_t &offset, size_t dataSize,
                       std::string &out) {
  uint32_t len = 0;
  if (!Read(data, offset, dataSize, len))
    return false;
  if (len == 0 || len > 1024 || offset + len > dataSize)
    return false;
  out.assign(reinterpret_cast<const char *>(data + offset), len);
  offset += len;
  return true;
}

// ===================================================================
// MAIN LOAD FUNCTIONS
// ===================================================================
bool C3PhyLoader::Load(const std::string &filepath, C3Phy &outPhy) {
  std::ifstream file(filepath, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    YAMEN_CORE_ERROR("C3PhyLoader: Failed to open file: {}", filepath);
    return false;
  }

  size_t fileSize = static_cast<size_t>(file.tellg());
  file.seekg(0, std::ios::beg);

  std::vector<uint8_t> buffer(fileSize);
  if (!file.read(reinterpret_cast<char *>(buffer.data()), fileSize)) {
    YAMEN_CORE_ERROR("C3PhyLoader: Failed to read file: {}", filepath);
    return false;
  }

  return LoadFromMemory(buffer.data(), fileSize, outPhy);
}

bool C3PhyLoader::LoadFromMemory(const uint8_t *data, size_t size,
                                 C3Phy &outPhy) {
  if (size < 16) {
    YAMEN_CORE_ERROR("C3PhyLoader: File too small");
    return false;
  }

  size_t offset = 0;
  bool firstMotionLoaded = false;

  // Skip MAXFILE header if present
  if (memcmp(data, "MAXFILE C3", 10) == 0) {
    YAMEN_CORE_INFO("C3PhyLoader: Detected MAXFILE header, skipping 16 bytes");
    offset = 16;
  }

  // Main chunk parsing loop
  while (offset + 8 < size) {
    char chunkID[4] = {0};
    uint32_t chunkSize = 0;

    if (!Read(data, offset, size, chunkID[0]) ||
        !Read(data, offset, size, chunkID[1]) ||
        !Read(data, offset, size, chunkID[2]) ||
        !Read(data, offset, size, chunkID[3]) ||
        !Read(data, offset, size, chunkSize)) {
      break;
    }

    size_t chunkEnd = offset + chunkSize;
    if (chunkEnd > size) {
      YAMEN_CORE_WARN("C3PhyLoader: Chunk {} truncated ({} > {})",
                      std::string(chunkID, 4), chunkEnd, size);
      break;
    }

    if (memcmp(chunkID, "MOTN", 4) == 0 || memcmp(chunkID, "MOTI", 4) == 0) {
      if (!firstMotionLoaded) {
        YAMEN_CORE_INFO(
            "C3PhyLoader: Loading main skeleton from {} chunk ({} bytes)",
            std::string(chunkID, 4), chunkSize);
        if (!outPhy.motion) {
          outPhy.motion = new C3Motion();
        }
        if (!ParseMotionChunk(data, offset, size, *outPhy.motion)) {
          YAMEN_CORE_ERROR("C3PhyLoader: Failed to parse main motion chunk");
          return false;
        }
        firstMotionLoaded = true;
      } else {
        YAMEN_CORE_INFO(
            "C3PhyLoader: Skipping extra {} chunk (accessory animation)",
            std::string(chunkID, 4));
      }
    } else if (memcmp(chunkID, "PHYS", 4) == 0 ||
               memcmp(chunkID, "PHY ", 4) == 0 ||
               memcmp(chunkID, "PHY4", 4) == 0) {
      bool isPhy4 = (memcmp(chunkID, "PHY4", 4) == 0);
      YAMEN_CORE_INFO("C3PhyLoader: Loading physics mesh '{}' ({} bytes)",
                      std::string(chunkID, 4), chunkSize);
      if (!ParsePhysicsChunk(data, offset, size, outPhy, isPhy4)) {
        if (outPhy.vertices.empty()) {
          YAMEN_CORE_ERROR("C3PhyLoader: Failed to parse physics chunk and no "
                           "vertices loaded");
          return false;
        } else {
          YAMEN_CORE_WARN("C3PhyLoader: Partial physics load - continuing with "
                          "existing mesh");
        }
      }
    } else if (memcmp(chunkID, "MAXF", 4) == 0 ||
               memcmp(chunkID, "PTCL", 4) == 0) {
      YAMEN_CORE_INFO("C3PhyLoader: Skipping known chunk: {}",
                      std::string(chunkID, 4));
    } else {
      YAMEN_CORE_WARN("C3PhyLoader: Unknown chunk: {}{}{}{} ({} bytes)",
                      chunkID[0], chunkID[1], chunkID[2], chunkID[3],
                      chunkSize);
    }

    offset = chunkEnd;
  }

  // Calculate inverse bind pose from frame 0
  /*
  if (outPhy.motion && !outPhy.motion->keyframes.empty() &&
      outPhy.motion->boneCount > 0) {
    const auto &frame0 = outPhy.motion->keyframes[0];
    if (frame0.boneMatrices.size() == outPhy.motion->boneCount) {
      outPhy.invBindMatrices.resize(outPhy.motion->boneCount);
      for (size_t i = 0; i < outPhy.motion->boneCount; ++i) {
        outPhy.invBindMatrices[i] = glm::inverse(frame0.boneMatrices[i]);
      }
      YAMEN_CORE_INFO(
          "C3PhyLoader: Generated {} inverse bind pose matrices from frame 0",
          outPhy.invBindMatrices.size());
    }
  }
  */

  YAMEN_CORE_INFO("C3PhyLoader: Successfully loaded PHY");
  YAMEN_CORE_INFO("   Vertices: {} | Triangles: {} | Bones: {}",
                  outPhy.vertices.size(),
                  outPhy.indices.empty() ? 0 : outPhy.indices.size() / 3,
                  outPhy.motion ? outPhy.motion->boneCount : 0);

  return true;
}

// ===================================================================
// MOTION PARSING
// ===================================================================
bool C3PhyLoader::ParseMotionChunk(const uint8_t *data, size_t &offset,
                                   size_t fileSize, C3Motion &motion) {
  if (!Read(data, offset, fileSize, motion.boneCount))
    return false;
  if (!Read(data, offset, fileSize, motion.frameCount))
    return false;

  motion.currentBones.resize(motion.boneCount, glm::mat4(1.0f));
  motion.currentFrame = 0;

  char formatID[4] = {0};
  size_t formatOffset = offset;

  if (offset + 4 <= fileSize) {
    memcpy(formatID, data + offset, 4);
    offset += 4;
  }

  if (memcmp(formatID, "KKEY", 4) == 0) {
    motion.format = C3KeyframeFormat::KKEY;
    return ParseKeyframesKKEY(data, offset, fileSize, motion);
  } else if (memcmp(formatID, "XKEY", 4) == 0) {
    motion.format = C3KeyframeFormat::XKEY;
    return ParseKeyframesXKEY(data, offset, fileSize, motion);
  } else if (memcmp(formatID, "ZKEY", 4) == 0) {
    motion.format = C3KeyframeFormat::ZKEY;
    return ParseKeyframesZKEY(data, offset, fileSize, motion);
  } else {
    offset = formatOffset;
    motion.format = C3KeyframeFormat::Legacy;
    return ParseKeyframesLegacy(data, offset, fileSize, motion);
  }
}

bool C3PhyLoader::ParseKeyframesKKEY(const uint8_t *data, size_t &offset,
                                     size_t fileSize, C3Motion &motion) {
  if (!Read(data, offset, fileSize, motion.keyframeCount))
    return false;
  motion.keyframes.resize(motion.keyframeCount);

  for (uint32_t i = 0; i < motion.keyframeCount; ++i) {
    auto &kf = motion.keyframes[i];
    if (!Read(data, offset, fileSize, kf.framePosition))
      return false;
    kf.boneMatrices.resize(motion.boneCount);
    for (uint32_t b = 0; b < motion.boneCount; ++b) {
      if (!Read(data, offset, fileSize, kf.boneMatrices[b]))
        return false;
      kf.boneMatrices[b] = glm::transpose(kf.boneMatrices[b]);
    }
  }
  YAMEN_CORE_INFO("C3PhyLoader: Loaded {} KKEY keyframes",
                  motion.keyframeCount);
  return true;
}

bool C3PhyLoader::ParseKeyframesXKEY(const uint8_t *data, size_t &offset,
                                     size_t fileSize, C3Motion &motion) {
  if (!Read(data, offset, fileSize, motion.keyframeCount))
    return false;
  motion.keyframes.resize(motion.keyframeCount);

  for (uint32_t i = 0; i < motion.keyframeCount; ++i) {
    auto &kf = motion.keyframes[i];
    uint16_t frame16;
    if (!Read(data, offset, fileSize, frame16))
      return false;
    kf.framePosition = frame16;
    kf.boneMatrices.resize(motion.boneCount);
    for (uint32_t b = 0; b < motion.boneCount; ++b) {
      TidyMatrix tidy;
      if (!Read(data, offset, fileSize, tidy))
        return false;
      kf.boneMatrices[b] = tidy.ToMat4();
    }
  }
  YAMEN_CORE_INFO("C3PhyLoader: Loaded {} XKEY keyframes",
                  motion.keyframeCount);
  return true;
}

bool C3PhyLoader::ParseKeyframesZKEY(const uint8_t *data, size_t &offset,
                                     size_t fileSize, C3Motion &motion) {
  if (!Read(data, offset, fileSize, motion.keyframeCount))
    return false;
  motion.keyframes.resize(motion.keyframeCount);

  for (uint32_t i = 0; i < motion.keyframeCount; ++i) {
    auto &kf = motion.keyframes[i];
    uint16_t frame16;
    if (!Read(data, offset, fileSize, frame16))
      return false;
    kf.framePosition = frame16;
    kf.boneMatrices.resize(motion.boneCount);
    for (uint32_t b = 0; b < motion.boneCount; ++b) {
      DivInfo div;
      if (!Read(data, offset, fileSize, div))
        return false;
      kf.boneMatrices[b] = div.ToMat4();
    }
  }
  YAMEN_CORE_INFO("C3PhyLoader: Loaded {} ZKEY keyframes",
                  motion.keyframeCount);
  return true;
}

bool C3PhyLoader::ParseKeyframesLegacy(const uint8_t *data, size_t &offset,
                                       size_t fileSize, C3Motion &motion) {
  motion.keyframeCount = motion.frameCount;
  motion.keyframes.resize(motion.keyframeCount);

  for (uint32_t f = 0; f < motion.frameCount; ++f) {
    auto &kf = motion.keyframes[f];
    kf.framePosition = f;
    kf.boneMatrices.resize(motion.boneCount);
    for (uint32_t b = 0; b < motion.boneCount; ++b) {
      // Legacy C3 matrices are 3x4 (48 bytes), NOT 4x4 (64 bytes)
      // We must read 12 floats and construct the matrix
      struct Matrix3x4 {
        float m[12];
      } mat3x4;

      if (!Read(data, offset, fileSize, mat3x4)) {
        YAMEN_CORE_ERROR("Legacy parse failed at frame {} bone {}", f, b);
        return false;
      }

      // Construct 4x4 matrix from 3x4 data
      // Assuming Row-Major storage in file:
      // Row 0: m[0], m[1], m[2], m[3]
      // Row 1: m[4], m[5], m[6], m[7]
      // Row 2: m[8], m[9], m[10], m[11]
      // Row 3: 0, 0, 0, 1

      // GLM is Column-Major, so we construct it carefully.
      // If the file is Row-Major 3x4 (standard D3D9):
      // We can load it into a glm::mat4 and then Transpose it?
      // Or construct it directly.
      // Let's construct a temporary row-major matrix and transpose it to be
      // safe/standard.

      glm::mat4 m(1.0f);
      // Row 0
      m[0][0] = mat3x4.m[0];
      m[0][1] = mat3x4.m[1];
      m[0][2] = mat3x4.m[2];
      m[0][3] = mat3x4.m[3];
      // Row 1
      m[1][0] = mat3x4.m[4];
      m[1][1] = mat3x4.m[5];
      m[1][2] = mat3x4.m[6];
      m[1][3] = mat3x4.m[7];
      // Row 2
      m[2][0] = mat3x4.m[8];
      m[2][1] = mat3x4.m[9];
      m[2][2] = mat3x4.m[10];
      m[2][3] = mat3x4.m[11];

      // Current 'm' has Translation in Column 0 (m[0][3]).
      // We need Translation in Column 3.
      // glm::transpose swaps Rows and Cols, moving Row 3 (Tx,Ty,Tz,1) to Col 3.
      kf.boneMatrices[b] = glm::transpose(m);
    }
  }
  YAMEN_CORE_INFO("C3PhyLoader: Loaded {} legacy keyframes",
                  motion.keyframeCount);
  return true;
}

// ===================================================================
// PHYSICS MESH PARSING
// ===================================================================
uint32_t C3PhyLoader::GetBoneIndexForMesh(const std::string &name) {
  std::string lower = name;
  std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

  if (lower.find("head") != std::string::npos ||
      lower.find("helmet") != std::string::npos ||
      lower.find("armet") != std::string::npos)
    return 15;
  if (lower.find("l_weapon") != std::string::npos ||
      lower.find("l_shield") != std::string::npos ||
      lower.find("l_hand") != std::string::npos)
    return 25;
  if (lower.find("r_weapon") != std::string::npos ||
      lower.find("r_shield") != std::string::npos ||
      lower.find("r_hand") != std::string::npos)
    return 45;
  if (lower.find("l_foot") != std::string::npos ||
      lower.find("l_shoe") != std::string::npos)
    return 5;
  if (lower.find("r_foot") != std::string::npos ||
      lower.find("r_shoe") != std::string::npos)
    return 10;
  if (lower.find("back") != std::string::npos ||
      lower.find("mantle") != std::string::npos ||
      lower.find("cape") != std::string::npos)
    return 1;

  return 0; // Root
}

bool C3PhyLoader::ParsePhysicsChunk(const uint8_t *data, size_t &offset,
                                    size_t fileSize, C3Phy &phy, bool isPhy4) {
  uint32_t nameLen = 0;
  if (!Read(data, offset, fileSize, nameLen))
    return false;

  std::string name = "unnamed";
  if (nameLen > 0 && nameLen < 256 && offset + nameLen <= fileSize) {
    name.assign(reinterpret_cast<const char *>(data + offset), nameLen);
    offset += nameLen;
  }
  YAMEN_CORE_INFO("   Mesh name: '{}'", name);

  uint32_t blendCount = 0;
  if (!Read(data, offset, fileSize, blendCount))
    return false;
  phy.blendCount = blendCount;

  if (!Read(data, offset, fileSize, phy.normalVertexCount))
    return false;
  if (!Read(data, offset, fileSize, phy.alphaVertexCount))
    return false;

  uint32_t totalVerts = phy.normalVertexCount + phy.alphaVertexCount;
  if (totalVerts == 0 || totalVerts > 200000) {
    YAMEN_CORE_ERROR("   Invalid vertex count: {}", totalVerts);
    return false;
  }

  size_t baseVertIndex = phy.vertices.size();
  phy.vertices.resize(baseVertIndex + totalVerts);

  uint32_t attachBone = GetBoneIndexForMesh(name);

  if (isPhy4 || blendCount == 0) {
    // 40-byte fixed format: pos(12) + norm(12) + uv(8) + pad(8)
    const size_t stride = 40;
    if (offset + totalVerts * stride > fileSize)
      return false;

    for (uint32_t i = 0; i < totalVerts; ++i) {
      const uint8_t *v = data + offset;
      auto &vert = phy.vertices[baseVertIndex + i];
      memcpy(&vert.position, v + 0, 12);
      memcpy(&vert.normal, v + 12, 12);
      memcpy(&vert.texCoord, v + 24, 8);

      vert.boneIndices[0] = attachBone;
      vert.boneIndices[1] = vert.boneIndices[2] = vert.boneIndices[3] = 0;
      vert.boneWeights[0] = 1.0f;
      vert.boneWeights[1] = vert.boneWeights[2] = vert.boneWeights[3] = 0.0f;

      offset += stride;
    }
  } else {
    // Blended format
    size_t stride = blendCount * 4 + blendCount * 4 + 12 + 12;
    if (offset + totalVerts * stride > fileSize)
      return false;

    for (uint32_t i = 0; i < totalVerts; ++i) {
      const uint8_t *v = data + offset;
      auto &vert = phy.vertices[baseVertIndex + i];

      for (uint32_t b = 0; b < blendCount && b < 4; ++b)
        vert.boneIndices[b] = reinterpret_cast<const uint32_t *>(v)[b];
      for (uint32_t b = blendCount; b < 4; ++b)
        vert.boneIndices[b] = 0;

      const float *w = reinterpret_cast<const float *>(v + blendCount * 4);
      for (uint32_t b = 0; b < blendCount && b < 4; ++b)
        vert.boneWeights[b] = w[b];
      for (uint32_t b = blendCount; b < 4; ++b)
        vert.boneWeights[b] = 0.0f;

      memcpy(&vert.position, v + blendCount * 8, 12);
      memcpy(&vert.normal, v + blendCount * 8 + 12, 12);
      vert.texCoord = glm::vec2(0.0f);

      offset += stride;
    }
  }

  // Triangles
  if (!Read(data, offset, fileSize, phy.normalTriCount))
    return false;
  if (!Read(data, offset, fileSize, phy.alphaTriCount))
    return false;

  uint32_t totalTris = phy.normalTriCount + phy.alphaTriCount;
  size_t baseIndex = phy.indices.size();
  phy.indices.resize(baseIndex + totalTris * 3);

  for (uint32_t i = 0; i < totalTris * 3; ++i) {
    uint16_t idx;
    if (!Read(data, offset, fileSize, idx))
      return false;
    phy.indices[baseIndex + i] = idx + static_cast<uint16_t>(baseVertIndex);
  }

  ReadString(data, offset, fileSize, phy.textureName);

  YAMEN_CORE_INFO("   Loaded {} verts, {} tris, attached to bone {}",
                  totalVerts, totalTris, attachBone);
  return true;
}

// ===================================================================
// INTERPOLATION
// ===================================================================
void C3PhyLoader::InterpolateBones(const C3Motion &motion, float frame,
                                   std::vector<glm::mat4> &outMatrices) {
  if (motion.boneCount == 0 || motion.keyframes.empty()) {
    outMatrices.assign(motion.boneCount, glm::mat4(1.0f));
    return;
  }

  outMatrices.resize(motion.boneCount);

  float maxFrame = motion.keyframes.back().framePosition;
  frame = glm::clamp(frame, 0.0f, maxFrame);

  size_t kf1 = 0, kf2 = 0;
  for (size_t i = 0; i < motion.keyframes.size(); ++i) {
    if (motion.keyframes[i].framePosition <= frame)
      kf1 = i;
    if (motion.keyframes[i].framePosition >= frame) {
      kf2 = i;
      break;
    }
  }
  if (kf2 >= motion.keyframes.size())
    kf2 = kf1;

  float t = 0.0f;
  if (kf1 != kf2) {
    float f1 = static_cast<float>(motion.keyframes[kf1].framePosition);
    float f2 = static_cast<float>(motion.keyframes[kf2].framePosition);
    if (f2 > f1)
      t = (frame - f1) / (f2 - f1);
  }

  for (uint32_t b = 0; b < motion.boneCount; ++b) {
    const glm::mat4 &m1 = motion.keyframes[kf1].boneMatrices[b];
    const glm::mat4 &m2 = motion.keyframes[kf2].boneMatrices[b];

    glm::vec3 pos1, pos2, scale1, scale2, skew;
    glm::quat rot1, rot2;
    glm::vec4 persp;

    glm::decompose(m1, scale1, rot1, pos1, skew, persp);
    glm::decompose(m2, scale2, rot2, pos2, skew, persp);

    if (glm::dot(rot1, rot2) < 0.0f)
      rot2 = -rot2;

    glm::quat rot = glm::slerp(rot1, rot2, t);
    glm::vec3 pos = glm::mix(pos1, pos2, t);
    glm::vec3 scale = glm::mix(scale1, scale2, t);

    outMatrices[b] = glm::translate(glm::mat4(1.0f), pos) *
                     glm::mat4_cast(rot) * glm::scale(glm::mat4(1.0f), scale);
  }
}
bool C3PhyLoader::ReadString(const uint8_t *data, size_t &offset,
                             size_t dataSize, std::string &out) {
  uint32_t len = 0;
  if (!Read(data, offset, dataSize, len))
    return false;
  if (len == 0 || len > 1024 || offset + len > dataSize)
    return false;
  out.assign(reinterpret_cast<const char *>(data + offset), len);
  offset += len;
  return true;
}
} // namespace Assets
} // namespace Yamen
