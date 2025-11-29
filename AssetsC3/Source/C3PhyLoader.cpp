#include "AssetsC3/C3PhyLoader.h"
#include "Core/Logging/Logger.h"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <vector>

namespace Yamen {
namespace Assets {

using namespace Yamen::Core;

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

  // Skip MAXFILE header
  if (memcmp(data, "MAXFILE C3", 10) == 0) {
    YAMEN_CORE_INFO("C3PhyLoader: Detected MAXFILE header, skipping 16 bytes");
    offset = 16;
  }

  // --- Chunk Parsing Loop ---
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
      YAMEN_CORE_WARN("C3PhyLoader: Chunk {} truncated",
                      std::string(chunkID, 4));
      break;
    }

    if (memcmp(chunkID, "MOTN", 4) == 0 || memcmp(chunkID, "MOTI", 4) == 0) {
      if (!firstMotionLoaded) {
        YAMEN_CORE_INFO("C3PhyLoader: Loading main skeleton from {}",
                        std::string(chunkID, 4));
        if (!outPhy.motion)
          outPhy.motion = new C3Motion();
        if (!ParseMotionChunk(data, offset, size, *outPhy.motion))
          return false;
        firstMotionLoaded = true;
      }
    } else if (memcmp(chunkID, "PHYS", 4) == 0 ||
               memcmp(chunkID, "PHY ", 4) == 0 ||
               memcmp(chunkID, "PHY4", 4) == 0) {
      bool isPhy4 = (memcmp(chunkID, "PHY4", 4) == 0);
      YAMEN_CORE_INFO("C3PhyLoader: Loading physics mesh '{}'",
                      std::string(chunkID, 4));
      ParsePhysicsChunk(data, offset, size, outPhy, isPhy4);
    }

    offset = chunkEnd;
  }

  /// =============================================================
  // FIX: CALCULATE INVERSE BIND POSE (SANITIZED)
  // =============================================================
  if (outPhy.motion && !outPhy.motion->keyframes.empty() &&
      outPhy.motion->boneCount > 0) {

    const auto &frame0 = outPhy.motion->keyframes[0];
    outPhy.invBindMatrices.resize(outPhy.motion->boneCount);

    for (size_t i = 0; i < outPhy.motion->boneCount; ++i) {
      mat4 bm = frame0.boneMatrices[i];

      // Manual Scale Stripping (Extract Columns/Rows)
      // In Row-Major, m[0] is Row 0 (Right), m[1] is Row 1 (Up), m[2] is Row 2
      // (Fwd)
      vec3 right = vec3(bm[0]);
      vec3 up = vec3(bm[1]);
      vec3 fwd = vec3(bm[2]);
      vec3 pos = vec3(bm[3]);

      // Safe Normalize
      if (Math::Length(right) > 0.0001f)
        right = Math::Normalize(right);
      else
        right = vec3(1, 0, 0);
      if (Math::Length(up) > 0.0001f)
        up = Math::Normalize(up);
      else
        up = vec3(0, 1, 0);
      if (Math::Length(fwd) > 0.0001f)
        fwd = Math::Normalize(fwd);
      else
        fwd = vec3(0, 0, 1);

      // Reconstruct sanitized matrix
      mat4 sanitized(vec4(right, 0.0f), vec4(up, 0.0f), vec4(fwd, 0.0f),
                     vec4(pos, 1.0f));

      // Invert
      outPhy.invBindMatrices[i] = Math::Inverse(sanitized);
    }
  }

  YAMEN_CORE_INFO("C3PhyLoader: Successfully loaded PHY");
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

  motion.currentBones.resize(motion.boneCount, mat4(1.0f));
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
      // FIX: KKEY is 3x4 (48 bytes) like Legacy
      struct Matrix3x4 {
        float m[12];
      } mat3x4;
      if (!Read(data, offset, fileSize, mat3x4))
        return false;

      mat4 m(vec4(mat3x4.m[0], mat3x4.m[1], mat3x4.m[2], mat3x4.m[3]),
             vec4(mat3x4.m[4], mat3x4.m[5], mat3x4.m[6], mat3x4.m[7]),
             vec4(mat3x4.m[8], mat3x4.m[9], mat3x4.m[10], mat3x4.m[11]),
             vec4(0.0f, 0.0f, 0.0f, 1.0f));

      kf.boneMatrices[b] = m;
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

      // TidyMatrix::ToMat4() already returns a correct mat4
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

      // DivInfo::ToMat4() already returns a correct mat4
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
      struct Matrix3x4 {
        float m[12];
      } mat3x4;

      if (!Read(data, offset, fileSize, mat3x4)) {
        YAMEN_CORE_ERROR("Legacy parse failed at frame {} bone {}", f, b);
        return false;
      }

      mat4 m(vec4(mat3x4.m[0], mat3x4.m[1], mat3x4.m[2], mat3x4.m[3]),
             vec4(mat3x4.m[4], mat3x4.m[5], mat3x4.m[6], mat3x4.m[7]),
             vec4(mat3x4.m[8], mat3x4.m[9], mat3x4.m[10], mat3x4.m[11]),
             vec4(0.0f, 0.0f, 0.0f, 1.0f));

      kf.boneMatrices[b] = m;
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

  // FILTER: Skip unwanted meshes
  std::string lowerName = name;
  std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
                 ::tolower);

  if (lowerName.find("box") != std::string::npos ||
      lowerName.find("bound") != std::string::npos ||
      lowerName.find("shadow") != std::string::npos ||
      lowerName.find("collision") != std::string::npos ||
      lowerName.find("dummy") != std::string::npos) {
    YAMEN_CORE_WARN("   Skipping auxiliary mesh: '{}'", name);
    return true;
  }

  YAMEN_CORE_INFO("   Loading Mesh: '{}'", name);

  uint32_t blendCount = 0;
  if (!Read(data, offset, fileSize, blendCount))
    return false;
  phy.blendCount = blendCount;

  YAMEN_CORE_INFO("   BlendCount: {}", blendCount);

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

  if (blendCount == 0) {
    // Rigid vertices: 40-byte format
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
    // Blended vertices: 40-byte format (CompactVertex)
    // Reference Layout: Pos(12) | Normal(12) | UV(8) | BoneIdx(4) | Color(4)
    const size_t stride = 40;
    YAMEN_CORE_INFO("   Blended vertex stride: {} bytes (Compact Format)",
                    stride);

    if (offset + totalVerts * stride > fileSize) {
      YAMEN_CORE_ERROR("   Not enough data for blended vertices!");
      return false;
    }

    struct CompactVertex {
      float x, y, z;    // Pos
      float nx, ny, nz; // Normal
      float u, v;       // UV
      uint32_t boneIdx; // Packed Bone Indices
      uint32_t color;   // Color
    };

    for (uint32_t i = 0; i < totalVerts; ++i) {
      CompactVertex cv;
      if (!Read(data, offset, fileSize, cv))
        return false;

      auto &vert = phy.vertices[baseVertIndex + i];

      // Position
      vert.position = vec3(cv.x, cv.y, cv.z);

      // Normal
      vert.normal = vec3(cv.nx, cv.ny, cv.nz);

      // UV
      vert.texCoord = vec2(cv.u, cv.v);

      // Bones (Unpack from boneIdx)
      // Reference code: v.boneIndices[0] = cv.boneIdx & 0xFF;
      //                 v.boneIndices[1] = (cv.boneIdx >> 8) & 0xFF;
      vert.boneIndices[0] = cv.boneIdx & 0xFF;
      vert.boneIndices[1] = (cv.boneIdx >> 8) & 0xFF;
      vert.boneIndices[2] = 0;
      vert.boneIndices[3] = 0;

      // Weights
      // Reference code sets w[0]=1.0, w[1]=0.0.
      // But since we have 2 bones, we should probably distribute weights?
      // For now, let's stick to reference to get the SHAPE right.
      // If the mesh distorts, we can tweak weights later.
      // Actually, if blendCount is 2, we expect 2 weights.
      // If the file only stores indices, maybe weights are implicit (0.5/0.5)?
      // Or maybe the reference code is just for rigid meshes.
      // Let's try 1.0/0.0 for now to ensure stability, or 0.5/0.5 if both
      // indices are valid.

      if (vert.boneIndices[1] != 0 &&
          vert.boneIndices[1] != vert.boneIndices[0]) {
        vert.boneWeights[0] = 0.5f;
        vert.boneWeights[1] = 0.5f;
      } else {
        vert.boneWeights[0] = 1.0f;
        vert.boneWeights[1] = 0.0f;
      }
      vert.boneWeights[2] = 0.0f;
      vert.boneWeights[3] = 0.0f;

      // Debug first vertex
      if (i == 0) {
        YAMEN_CORE_INFO("   First vertex: Bones=[{},{}], "
                        "Weights=[{:.2f},{:.2f}], Pos=[{:.2f},{:.2f},{:.2f}]",
                        vert.boneIndices[0], vert.boneIndices[1],
                        vert.boneWeights[0], vert.boneWeights[1],
                        vert.position.x, vert.position.y, vert.position.z);
      }
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
                                   std::vector<mat4> &outMatrices) {
  if (motion.boneCount == 0 || motion.keyframes.empty()) {
    outMatrices.assign(motion.boneCount, mat4(1.0f));
    return;
  }

  outMatrices.resize(motion.boneCount);

  float maxFrame = motion.keyframes.back().framePosition;
  frame = Math::Clamp(frame, 0.0f, maxFrame);

  // Find keyframes
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

  // Interpolation factor
  float t = 0.0f;
  if (kf1 != kf2) {
    float f1 = static_cast<float>(motion.keyframes[kf1].framePosition);
    float f2 = static_cast<float>(motion.keyframes[kf2].framePosition);
    if (f2 > f1)
      t = (frame - f1) / (f2 - f1);
  }

  // Interpolate
  for (uint32_t b = 0; b < motion.boneCount; ++b) {
    const mat4 &m1 = motion.keyframes[kf1].boneMatrices[b];
    const mat4 &m2 = motion.keyframes[kf2].boneMatrices[b];

    vec3 pos1 = vec3(m1[3]);
    vec3 pos2 = vec3(m2[3]);

    auto GetRot = [](const mat4 &m) {
      vec3 r = vec3(m[0]);
      vec3 u = vec3(m[1]);
      vec3 f = vec3(m[2]);

      if (Math::Length(r) > 1e-5f)
        r = Math::Normalize(r);
      else
        r = vec3(1, 0, 0);
      if (Math::Length(u) > 1e-5f)
        u = Math::Normalize(u);
      else
        u = vec3(0, 1, 0);
      if (Math::Length(f) > 1e-5f)
        f = Math::Normalize(f);
      else
        f = vec3(0, 0, 1);

      mat4 rotMat(vec4(r, 0.0f), vec4(u, 0.0f), vec4(f, 0.0f),
                  vec4(0.0f, 0.0f, 0.0f, 1.0f));

      return Math::ToQuat(rotMat);
    };

    quat rot1 = GetRot(m1);
    quat rot2 = GetRot(m2);

    if (Math::Dot(rot1, rot2) < 0.0f)
      rot2 = -rot2;

    quat rot = Math::Slerp(rot1, rot2, t);
    vec3 pos = Math::Lerp(pos1, pos2, t);

    outMatrices[b] = Math::Translate(pos) * Math::ToMat4(rot);
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
