#include "AssetsC3/C3PhyLoader.h"
#include "Core/Logging/Logger.h"
#include <algorithm>
#include <cstring>
#include <fstream>

namespace Yamen::Assets {

bool C3PhyLoader::Load(const std::string &filepath, C3Phy &outPhy) {
  // Read entire file into memory
  std::ifstream file(filepath, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    YAMEN_CORE_ERROR("Failed to open PHY file: {}", filepath);
    return false;
  }

  size_t fileSize = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<uint8_t> buffer(fileSize);
  if (!file.read(reinterpret_cast<char *>(buffer.data()), fileSize)) {
    YAMEN_CORE_ERROR("Failed to read PHY file: {}", filepath);
    return false;
  }

  return LoadFromMemory(buffer.data(), fileSize, outPhy);
}

bool C3PhyLoader::LoadFromMemory(const uint8_t *data, size_t size,
                                 C3Phy &outPhy) {
  size_t offset = 0;

  // Check for MAXFILE header (3D Studio Max export format)
  // Format: "MAXFILE C3 00001" (16 bytes) + Chunks
  if (size >= 16 && memcmp(data, "MAXFILE C3", 10) == 0) {
    YAMEN_CORE_INFO("Detected MAXFILE (3DS Max) format");
    offset = 16; // Skip header, proceed to chunks
  }

  // Parse chunk-based format (original C3 files or MAXFILE chunk data)
  while (offset < size) {
    char chunkID[4];
    uint32_t chunkSize;

    if (!ReadChunkHeader(data, offset, size, chunkID, chunkSize)) {
      break; // End of file or error
    }

    size_t chunkEnd = offset + chunkSize;

    // Process chunk based on ID
    if (memcmp(chunkID, "MOTN", 4) == 0 || memcmp(chunkID, "MOTI", 4) == 0) {
      // Motion/Animation chunk (MOTN or MOTI)
      YAMEN_CORE_INFO("Found {} chunk, size: {} bytes", std::string(chunkID, 4),
                      chunkSize);
      if (!outPhy.motion) {
        outPhy.motion = new C3Motion();
      }
      if (!ParseMotionChunk(data, offset, chunkEnd, *outPhy.motion)) {
        YAMEN_CORE_ERROR("Failed to parse motion chunk");
        return false;
      }
    } else if (memcmp(chunkID, "PHYS", 4) == 0 ||
               memcmp(chunkID, "PHY ", 4) == 0) {
      // Physics/Mesh chunk (PHYS or "PHY ")
      YAMEN_CORE_INFO("Found {} chunk, size: {} bytes", std::string(chunkID, 4),
                      chunkSize);
      if (!ParsePhysicsChunk(data, offset, chunkEnd, outPhy)) {
        YAMEN_CORE_ERROR("Failed to parse physics chunk");
        return false;
      }
    } else if (memcmp(chunkID, "MAXF", 4) == 0) {
      // MAXF chunk - unknown format, skip for now
      YAMEN_CORE_WARN(
          "Found MAXF chunk, size: {} bytes - skipping (unknown format)",
          chunkSize);
      offset = chunkEnd;
    } else {
      // Unknown chunk, skip it
      YAMEN_CORE_WARN("Unknown chunk ID: {}{}{}{}, size: {} bytes", chunkID[0],
                      chunkID[1], chunkID[2], chunkID[3], chunkSize);
      offset = chunkEnd;
    }

    // Ensure we're at chunk boundary
    offset = chunkEnd;
  }

  YAMEN_CORE_INFO(
      "Successfully loaded PHY file: {} vertices, {} triangles, {} bones",
      outPhy.vertices.size(), outPhy.indices.size() / 3,
      outPhy.motion ? outPhy.motion->boneCount : 0);

  return true;
}

bool C3PhyLoader::ParseMotionChunk(const uint8_t *data, size_t &offset,
                                   size_t dataSize, C3Motion &motion) {
  // Read bone count and frame count
  if (!Read(data, offset, dataSize, motion.boneCount))
    return false;
  if (!Read(data, offset, dataSize, motion.frameCount))
    return false;

  YAMEN_CORE_INFO("Motion: {} bones, {} frames", motion.boneCount,
                  motion.frameCount);

  // Check for keyframe format identifier
  char formatID[4];
  size_t formatOffset = offset;
  if (Read(data, offset, dataSize, formatID)) {
    if (memcmp(formatID, "KKEY", 4) == 0) {
      motion.format = C3KeyframeFormat::KKEY;
      if (!ParseKeyframesKKEY(data, offset, motion))
        return false;
    } else if (memcmp(formatID, "XKEY", 4) == 0) {
      motion.format = C3KeyframeFormat::XKEY;
      if (!ParseKeyframesXKEY(data, offset, motion))
        return false;
    } else if (memcmp(formatID, "ZKEY", 4) == 0) {
      motion.format = C3KeyframeFormat::ZKEY;
      if (!ParseKeyframesZKEY(data, offset, motion))
        return false;
    } else {
      // Legacy format (no format ID, just raw matrices)
      offset = formatOffset; // Rewind
      motion.format = C3KeyframeFormat::Legacy;
      if (!ParseKeyframesLegacy(data, offset, motion))
        return false;
    }
  }

  // Read morph target data if present
  if (offset < dataSize) {
    if (Read(data, offset, dataSize, motion.morphCount)) {
      if (motion.morphCount > 0) {
        size_t morphDataSize = motion.morphCount * motion.frameCount;
        motion.morphWeights.resize(morphDataSize);
        for (size_t i = 0; i < morphDataSize; ++i) {
          if (!Read(data, offset, dataSize, motion.morphWeights[i])) {
            YAMEN_CORE_WARN("Incomplete morph data");
            break;
          }
        }
        YAMEN_CORE_INFO("Loaded {} morph targets", motion.morphCount);
      }
    }
  }

  // Initialize current bone matrices
  motion.currentBones.resize(motion.boneCount, glm::mat4(1.0f));
  motion.currentFrame = 0;

  return true;
}

bool C3PhyLoader::ParseKeyframesKKEY(const uint8_t *data, size_t &offset,
                                     C3Motion &motion) {
  // KKEY: Full 4x4 matrices per keyframe
  size_t dataSize = SIZE_MAX; // Use max size for non-bounded reads
  if (!Read(data, offset, dataSize, motion.keyframeCount))
    return false;

  motion.keyframes.resize(motion.keyframeCount);

  for (uint32_t i = 0; i < motion.keyframeCount; ++i) {
    auto &kf = motion.keyframes[i];

    // Read frame position
    if (!Read(data, offset, dataSize, kf.framePosition))
      return false;

    // Read bone matrices (full 4x4)
    kf.boneMatrices.resize(motion.boneCount);
    for (uint32_t b = 0; b < motion.boneCount; ++b) {
      if (!Read(data, offset, dataSize, kf.boneMatrices[b]))
        return false;
    }
  }

  YAMEN_CORE_INFO("Loaded {} KKEY keyframes", motion.keyframeCount);
  return true;
}

bool C3PhyLoader::ParseKeyframesXKEY(const uint8_t *data, size_t &offset,
                                     C3Motion &motion) {
  // XKEY: Compressed 3x4 matrices (12 floats instead of 16)
  size_t dataSize = SIZE_MAX;
  if (!Read(data, offset, dataSize, motion.keyframeCount))
    return false;

  motion.keyframes.resize(motion.keyframeCount);

  for (uint32_t i = 0; i < motion.keyframeCount; ++i) {
    auto &kf = motion.keyframes[i];

    // Read frame position (16-bit in XKEY)
    uint16_t framePos16;
    if (!Read(data, offset, dataSize, framePos16))
      return false;
    kf.framePosition = framePos16;

    // Read compressed bone matrices
    kf.boneMatrices.resize(motion.boneCount);
    for (uint32_t b = 0; b < motion.boneCount; ++b) {
      TidyMatrix tidyMat;
      if (!Read(data, offset, dataSize, tidyMat))
        return false;
      kf.boneMatrices[b] = tidyMat.ToMat4();
    }
  }

  YAMEN_CORE_INFO("Loaded {} XKEY keyframes", motion.keyframeCount);
  return true;
}

bool C3PhyLoader::ParseKeyframesZKEY(const uint8_t *data, size_t &offset,
                                     C3Motion &motion) {
  // ZKEY: Quaternion + translation (maximum compression)
  size_t dataSize = SIZE_MAX;
  if (!Read(data, offset, dataSize, motion.keyframeCount))
    return false;

  motion.keyframes.resize(motion.keyframeCount);

  for (uint32_t i = 0; i < motion.keyframeCount; ++i) {
    auto &kf = motion.keyframes[i];

    // Read frame position (16-bit)
    uint16_t framePos16;
    if (!Read(data, offset, dataSize, framePos16))
      return false;
    kf.framePosition = framePos16;

    // Read quaternion + translation per bone
    kf.boneMatrices.resize(motion.boneCount);
    for (uint32_t b = 0; b < motion.boneCount; ++b) {
      DivInfo divInfo;
      if (!Read(data, offset, dataSize, divInfo))
        return false;
      kf.boneMatrices[b] = divInfo.ToMat4();
    }
  }

  YAMEN_CORE_INFO("Loaded {} ZKEY keyframes", motion.keyframeCount);
  return true;
}

bool C3PhyLoader::ParseKeyframesLegacy(const uint8_t *data, size_t &offset,
                                       C3Motion &motion) {
  // Legacy: Full matrices stored per-bone-per-frame
  // Create keyframes for each frame
  motion.keyframeCount = motion.frameCount;
  motion.keyframes.resize(motion.keyframeCount);

  for (uint32_t f = 0; f < motion.frameCount; ++f) {
    auto &kf = motion.keyframes[f];
    kf.framePosition = f;
    kf.boneMatrices.resize(motion.boneCount);

    for (uint32_t b = 0; b < motion.boneCount; ++b) {
      size_t dataSize = SIZE_MAX;
      if (!Read(data, offset, dataSize, kf.boneMatrices[b])) {
        YAMEN_CORE_ERROR(
            "Failed to read legacy bone matrix at frame {}, bone {}", f, b);
        return false;
      }
    }
  }

  YAMEN_CORE_INFO("Loaded {} legacy keyframes", motion.keyframeCount);
  return true;
}

bool C3PhyLoader::ParsePhysicsChunk(const uint8_t *data, size_t &offset,
                                    size_t dataSize, C3Phy &phy) {
  size_t start_offset = offset;
  YAMEN_CORE_INFO("=== ParsePhysicsChunk START ===");
  YAMEN_CORE_INFO("  Start offset: {}, Data size: {}, Remaining: {}", offset,
                  dataSize, dataSize - offset);

  // Read name string
  uint32_t nameLen;
  if (!Read(data, offset, dataSize, nameLen)) {
    YAMEN_CORE_ERROR("  Failed to read name length");
    return false;
  }
  YAMEN_CORE_INFO("  Name length: {}", nameLen);

  std::string name;
  if (nameLen > 0 && nameLen < 256) {
    name.resize(nameLen);
    memcpy(&name[0], data + offset, nameLen);
    offset += nameLen;
  }
  YAMEN_CORE_INFO("  Model name: '{}', Offset now: {}", name, offset);

  // Read blend count
  uint32_t blendCount;
  if (!Read(data, offset, dataSize, blendCount)) {
    YAMEN_CORE_ERROR("  Failed to read blend count");
    return false;
  }
  phy.blendCount = blendCount;
  YAMEN_CORE_INFO("  Blend count: {}, Offset now: {}", blendCount, offset);

  // Read vertex and triangle counts
  if (!Read(data, offset, dataSize, phy.normalVertexCount)) {
    YAMEN_CORE_ERROR("  Failed to read normal vertex count");
    return false;
  }
  if (!Read(data, offset, dataSize, phy.alphaVertexCount)) {
    YAMEN_CORE_ERROR("  Failed to read alpha vertex count");
    return false;
  }

  uint32_t totalVertices = phy.normalVertexCount + phy.alphaVertexCount;

  YAMEN_CORE_INFO("  Vertices: {} normal, {} alpha, {} total",
                  phy.normalVertexCount, phy.alphaVertexCount, totalVertices);
  YAMEN_CORE_INFO("  Offset before vertices: {}", offset);

  // Calculate expected vertex data size
  // Calculate expected vertex data size
  size_t remainingBytes = dataSize - offset;
  size_t required76 = totalVertices * 76;
  bool use76ByteFormat = (remainingBytes >= required76);
  uint32_t expectedVertexBytes = 0;

  YAMEN_CORE_INFO(
      "  Vertex format detection: Remaining={}, Required76={}, Use76={}",
      remainingBytes, required76, use76ByteFormat);

  // Read vertices
  size_t baseVertexIndex = phy.vertices.size();
  phy.vertices.resize(baseVertexIndex + totalVertices);

  if (use76ByteFormat) {
    expectedVertexBytes = required76;
    // 76-byte format (Pos12 + Norm12 + Tan12 + UV8 + ? + Weights?)
    for (uint32_t i = 0; i < totalVertices; ++i) {
      auto &v = phy.vertices[baseVertexIndex + i];

      // Read 76 bytes
      if (offset + 76 > dataSize)
        return false;
      const uint8_t *vData = data + offset;

      // Extract fields
      memcpy(&v.position, vData, 12);     // 0-12
      memcpy(&v.normal, vData + 16, 12);  // 16-28 (skip 4 bytes padding/w)
      memcpy(&v.texCoord, vData + 48, 8); // 48-56

      // Default bone data
      v.boneIndices[0] = 0;
      v.boneWeights[0] = 1.0f;
      for (int b = 1; b < 4; ++b) {
        v.boneIndices[b] = 0;
        v.boneWeights[b] = 0.0f;
      }

      // Try to read weights from offset 68 (based on analysis of 1.0 float
      // value)
      float w0;
      memcpy(&w0, vData + 68, 4);
      if (w0 >= 0.0f && w0 <= 1.0f) {
        v.boneWeights[0] = w0;
      }

      offset += 76;
    }
  } else {
    // Original variable-size logic
    uint32_t vertexSize = 32 + blendCount + (blendCount * 4);
    expectedVertexBytes = totalVertices * vertexSize;
    YAMEN_CORE_INFO("  Expected vertex size: {} bytes each, {} total bytes",
                    vertexSize, expectedVertexBytes);

    for (uint32_t i = 0; i < totalVertices; ++i) {
      auto &v = phy.vertices[baseVertexIndex + i];

      if (!Read(data, offset, dataSize, v.position))
        return false;
      if (!Read(data, offset, dataSize, v.normal))
        return false;
      if (!Read(data, offset, dataSize, v.texCoord))
        return false;

      for (int b = 0; b < 4; ++b) {
        v.boneIndices[b] = 0;
        v.boneWeights[b] = 0.0f;
      }
      for (uint32_t b = 0; b < std::min(blendCount, 4u); ++b) {
        if (!Read(data, offset, dataSize, v.boneIndices[b]))
          return false;
      }
      for (uint32_t b = 0; b < std::min(blendCount, 4u); ++b) {
        if (!Read(data, offset, dataSize, v.boneWeights[b]))
          return false;
      }
    }
  }

  YAMEN_CORE_INFO("  Offset after vertices: {}, Bytes consumed: {}", offset,
                  offset - start_offset - 18);

  // Read triangle counts
  if (!Read(data, offset, dataSize, phy.normalTriCount)) {
    YAMEN_CORE_ERROR("  Failed to read normal triangle count at offset {}",
                     offset);
    return false;
  }
  if (!Read(data, offset, dataSize, phy.alphaTriCount)) {
    YAMEN_CORE_ERROR("  Failed to read alpha triangle count at offset {}",
                     offset);
    return false;
  }

  uint32_t totalTriangles = phy.normalTriCount + phy.alphaTriCount;
  YAMEN_CORE_INFO("  Triangles: {} normal, {} alpha, {} total",
                  phy.normalTriCount, phy.alphaTriCount, totalTriangles);

  // Sanity check triangle counts
  if (totalTriangles > 100000) {
    YAMEN_CORE_ERROR("  INVALID triangle count: {} (too large!)",
                     totalTriangles);
    YAMEN_CORE_ERROR(
        "  This suggests vertex parsing consumed incorrect number of bytes");
    YAMEN_CORE_ERROR(
        "  Expected {} vertex bytes, but offset suggests different alignment",
        expectedVertexBytes);
    return false;
  }

  // Read indices
  size_t baseIndexIndex = phy.indices.size();
  phy.indices.resize(baseIndexIndex + totalTriangles * 3);
  for (uint32_t i = 0; i < totalTriangles * 3; ++i) {
    uint16_t idx;
    if (!Read(data, offset, dataSize, idx))
      return false;
    phy.indices[baseIndexIndex + i] =
        idx + static_cast<uint16_t>(baseVertexIndex);
  }

  YAMEN_CORE_INFO("  Offset after indices: {}", offset);

  // Read texture name
  if (!ReadString(data, offset, dataSize, phy.textureName)) {
    YAMEN_CORE_WARN("  No texture name found");
  } else {
    YAMEN_CORE_INFO("  Texture: '{}'", phy.textureName);
  }

  // Read bounding box (MOVED TO END)
  glm::vec3 bboxMin, bboxMax;
  if (offset + 24 <= dataSize) {
    Read(data, offset, dataSize, bboxMin);
    Read(data, offset, dataSize, bboxMax);
    YAMEN_CORE_INFO("  Bounding box: min({:.2f}, {:.2f}, {:.2f}), max({:.2f}, "
                    "{:.2f}, {:.2f})",
                    bboxMin.x, bboxMin.y, bboxMin.z, bboxMax.x, bboxMax.y,
                    bboxMax.z);
  }

  // Read InitMatrix (MOVED TO END)
  glm::mat4 initMatrix;
  if (offset + 64 <= dataSize) {
    Read(data, offset, dataSize, initMatrix);
    YAMEN_CORE_INFO("  Read InitMatrix (64 bytes)");
  }

  // Read UV animation step
  if (Read(data, offset, dataSize, phy.uvAnimStep)) {
    if (phy.uvAnimStep.x != 0.0f || phy.uvAnimStep.y != 0.0f) {
      YAMEN_CORE_INFO("  UV animation: ({}, {})", phy.uvAnimStep.x,
                      phy.uvAnimStep.y);
    }
  }

  // Read texture rows
  Read(data, offset, dataSize, phy.textureRows);

  // Read material color (ARGB floats)
  float a, r, g, b;
  if (Read(data, offset, dataSize, a) && Read(data, offset, dataSize, r) &&
      Read(data, offset, dataSize, g) && Read(data, offset, dataSize, b)) {
    phy.color = glm::vec4(r, g, b, a);
  }

  YAMEN_CORE_INFO("=== ParsePhysicsChunk SUCCESS ===");
  YAMEN_CORE_INFO("  Total bytes consumed: {}", offset - start_offset);
  return true;
}

void C3PhyLoader::InterpolateBones(const C3Motion &motion, float frame,
                                   std::vector<glm::mat4> &outMatrices) {
  if (motion.keyframes.empty() || motion.boneCount == 0) {
    outMatrices.clear();
    return;
  }

  outMatrices.resize(motion.boneCount);

  // Clamp frame to valid range
  frame = std::max(0.0f,
                   std::min(frame, static_cast<float>(motion.frameCount - 1)));

  // Find surrounding keyframes
  size_t kf1 = 0, kf2 = 0;
  float t = 0.0f;

  for (size_t i = 0; i < motion.keyframes.size(); ++i) {
    if (motion.keyframes[i].framePosition <= frame) {
      kf1 = i;
    }
    if (motion.keyframes[i].framePosition >= frame) {
      kf2 = i;
      break;
    }
  }

  // Calculate interpolation factor
  if (kf1 != kf2) {
    float f1 = static_cast<float>(motion.keyframes[kf1].framePosition);
    float f2 = static_cast<float>(motion.keyframes[kf2].framePosition);
    t = (frame - f1) / (f2 - f1);
  }

  // Interpolate bone matrices
  for (uint32_t b = 0; b < motion.boneCount; ++b) {
    const glm::mat4 &m1 = motion.keyframes[kf1].boneMatrices[b];
    const glm::mat4 &m2 = motion.keyframes[kf2].boneMatrices[b];

    // Simple linear interpolation
    // For better results, decompose into TRS and interpolate separately
    outMatrices[b] = m1 * (1.0f - t) + m2 * t;
  }
}

bool C3PhyLoader::ReadString(const uint8_t *data, size_t &offset,
                             size_t dataSize, std::string &out) {
  uint32_t length;
  if (!Read(data, offset, dataSize, length))
    return false;

  if (length == 0 || length > 1024)
    return false; // Sanity check

  if (offset + length > dataSize)
    return false;

  out.assign(reinterpret_cast<const char *>(data + offset), length);
  offset += length;

  return true;
}

bool C3PhyLoader::ReadChunkHeader(const uint8_t *data, size_t &offset,
                                  size_t dataSize, char chunkID[4],
                                  uint32_t &chunkSize) {
  if (offset + 8 > dataSize)
    return false;

  memcpy(chunkID, data + offset, 4);
  offset += 4;

  memcpy(&chunkSize, data + offset, 4);
  offset += 4;

  return true;
}

} // namespace Yamen::Assets
