#include "AssetsC3/C3PhyLoader.h"
#include "Core/Logging/Logger.h"
#include <algorithm>
#include <cstring>
#include <fstream>

namespace Yamen {
    namespace Assets {

        bool C3PhyLoader::Load(const std::string& filepath, C3Phy& outPhy) {
            // Read entire file into memory
            std::ifstream file(filepath, std::ios::binary | std::ios::ate);
            if (!file.is_open()) {
                YAMEN_CORE_ERROR("Failed to open PHY file: {}", filepath);
                return false;
            }

            size_t fileSize = file.tellg();
            file.seekg(0, std::ios::beg);

            std::vector<uint8_t> buffer(fileSize);
            if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
                YAMEN_CORE_ERROR("Failed to read PHY file: {}", filepath);
                return false;
            }

            return LoadFromMemory(buffer.data(), fileSize, outPhy);
        }

        bool C3PhyLoader::LoadFromMemory(const uint8_t* data, size_t size,
            C3Phy& outPhy) {
            size_t offset = 0;

            // Check for MAXFILE header (3D Studio Max export format)
            // Format: "MAXFILE C3 00001" (16 bytes) + Chunks
            if (size >= 16 && memcmp(data, "MAXFILE C3", 10) == 0) {
                YAMEN_CORE_INFO("Detected MAXFILE (3DS Max) format");
                offset = 16; // Skip header, proceed to chunks
            }

            // Parse chunk-based format (original C3 files or MAXFILE chunk data)
            while (offset + 8 <= size) {
                char chunkID[4];
                uint32_t chunkSize;

                if (!ReadChunkHeader(data, offset, size, chunkID, chunkSize)) {
                    break; // End of file or error
                }

                size_t chunkEnd = offset + chunkSize;

                if (chunkEnd > size) {
                    YAMEN_CORE_WARN("Chunk {} exceeds file bounds (offset: {}, size: {}, file size: {}) - skipping",
                        std::string(chunkID, 4), offset, chunkSize, size);
                    break;
                }

                // Process chunk based on ID
                if (memcmp(chunkID, "MOTN", 4) == 0 || memcmp(chunkID, "MOTI", 4) == 0) {
                    YAMEN_CORE_INFO("Found {} chunk, size: {} bytes", std::string(chunkID, 4), chunkSize);
                    if (!outPhy.motion) {
                        outPhy.motion = new C3Motion();
                    }
                    if (!ParseMotionChunk(data, offset, chunkEnd, *outPhy.motion)) {
                        YAMEN_CORE_ERROR("Failed to parse motion chunk");
                        return false;
                    }
                }
                else if (memcmp(chunkID, "PHYS", 4) == 0 ||
                    memcmp(chunkID, "PHY ", 4) == 0 ||
                    memcmp(chunkID, "PHY4", 4) == 0) {
                    bool isPhy4 = (memcmp(chunkID, "PHY4", 4) == 0);
                    YAMEN_CORE_INFO("Found {} chunk, size: {} bytes", std::string(chunkID, 4), chunkSize);
                    if (!ParsePhysicsChunk(data, offset, chunkEnd, outPhy, isPhy4)) {
                        if (outPhy.vertices.size() > 0) {
                            YAMEN_CORE_WARN("Failed to parse secondary physics chunk - ignoring error to preserve loaded mesh");
                            offset = chunkEnd;
                        }
                        else {
                            YAMEN_CORE_ERROR("Failed to parse physics chunk");
                            return false;
                        }
                    }
                }
                else if (memcmp(chunkID, "MAXF", 4) == 0) {
                    YAMEN_CORE_WARN("Found MAXF chunk, size: {} bytes - skipping (unknown format)", chunkSize);
                    offset = chunkEnd;
                }
                else if (memcmp(chunkID, "PTCL", 4) == 0) {
                    YAMEN_CORE_WARN("Found PTCL chunk, size: {} bytes - skipping (unknown format)", chunkSize);
                    offset = chunkEnd;
                }
                else {
                    YAMEN_CORE_WARN("Unknown chunk ID: {}{}{}{}, size: {} bytes", chunkID[0],
                        chunkID[1], chunkID[2], chunkID[3], chunkSize);
                    offset = chunkEnd;
                }

                offset = chunkEnd;
            }

            YAMEN_CORE_INFO("Successfully loaded PHY file: {} vertices, {} triangles, {} bones",
                outPhy.vertices.size(), outPhy.indices.size() / 3,
                outPhy.motion ? outPhy.motion->boneCount : 0);

            // Calculate Inverse Bind Pose Matrices from Frame 0
            if (outPhy.motion && outPhy.motion->keyframeCount > 0 &&
                !outPhy.motion->keyframes[0].boneMatrices.empty()) {
                const auto& bindPose = outPhy.motion->keyframes[0].boneMatrices;
                if (bindPose.size() == outPhy.motion->boneCount) {
                    outPhy.invBindMatrices.resize(outPhy.motion->boneCount);
                    for (uint32_t i = 0; i < outPhy.motion->boneCount; ++i) {
                        outPhy.invBindMatrices[i] = glm::inverse(bindPose[i]);
                    }

                    const auto& m = outPhy.invBindMatrices[0];
                    YAMEN_CORE_INFO("Bone 0 InvBind: [{},{},{},{}] [{},{},{},{}] [{},{},{},{}] [{},{},{},{}]",
                        m[0][0], m[0][1], m[0][2], m[0][3], m[1][0], m[1][1], m[1][2], m[1][3],
                        m[2][0], m[2][1], m[2][2], m[2][3], m[3][0], m[3][1], m[3][2], m[3][3]);

                    YAMEN_CORE_INFO("Calculated {} Inverse Bind Matrices from Frame 0", outPhy.motion->boneCount);
                }
            }

            return true;
        }

        // FIXED: Now takes chunkEnd instead of dataSize
        bool C3PhyLoader::ParseMotionChunk(const uint8_t* data, size_t& offset,
            size_t chunkEnd, C3Motion& motion) {
            if (!Read(data, offset, chunkEnd, motion.boneCount)) return false;
            if (!Read(data, offset, chunkEnd, motion.frameCount)) return false;

            YAMEN_CORE_INFO("Motion: {} bones, {} frames", motion.boneCount, motion.frameCount);

            char formatID[4];
            size_t formatOffset = offset;
            if (Read(data, offset, chunkEnd, formatID)) {
                if (memcmp(formatID, "KKEY", 4) == 0) {
                    motion.format = C3KeyframeFormat::KKEY;
                    if (!ParseKeyframesKKEY(data, offset, chunkEnd, motion)) return false;
                }
                else if (memcmp(formatID, "XKEY", 4) == 0) {
                    motion.format = C3KeyframeFormat::XKEY;
                    if (!ParseKeyframesXKEY(data, offset, chunkEnd, motion)) return false;
                }
                else if (memcmp(formatID, "ZKEY", 4) == 0) {
                    motion.format = C3KeyframeFormat::ZKEY;
                    if (!ParseKeyframesZKEY(data, offset, chunkEnd, motion)) return false;
                }
                else {
                    offset = formatOffset;
                    motion.format = C3KeyframeFormat::Legacy;
                    if (!ParseKeyframesLegacy(data, offset, chunkEnd, motion)) return false;
                }
            }

            if (offset < chunkEnd && Read(data, offset, chunkEnd, motion.morphCount)) {
                if (motion.morphCount > 0) {
                    size_t morphDataSize = motion.morphCount * motion.frameCount;
                    motion.morphWeights.resize(morphDataSize);
                    for (size_t i = 0; i < morphDataSize && offset + 4 <= chunkEnd; ++i) {
                        if (!Read(data, offset, chunkEnd, motion.morphWeights[i])) {
                            YAMEN_CORE_WARN("Incomplete morph data");
                            break;
                        }
                    }
                    YAMEN_CORE_INFO("Loaded {} morph targets", motion.morphCount);
                }
            }

            motion.currentBones.resize(motion.boneCount, glm::mat4(1.0f));
            motion.currentFrame = 0;

            return true;
        }

        bool C3PhyLoader::ParseKeyframesKKEY(const uint8_t* data, size_t& offset,
            size_t chunkEnd, C3Motion& motion) {
            if (!Read(data, offset, chunkEnd, motion.keyframeCount)) return false;
            motion.keyframes.resize(motion.keyframeCount);

            for (uint32_t i = 0; i < motion.keyframeCount; ++i) {
                auto& kf = motion.keyframes[i];
                if (!Read(data, offset, chunkEnd, kf.framePosition)) return false;

                kf.boneMatrices.resize(motion.boneCount);
                for (uint32_t b = 0; b < motion.boneCount; ++b) {
                    if (!Read(data, offset, chunkEnd, kf.boneMatrices[b])) return false;
                    kf.boneMatrices[b] = glm::transpose(kf.boneMatrices[b]);
                }
            }

            YAMEN_CORE_INFO("Loaded {} KKEY keyframes", motion.keyframeCount);
            return true;
        }

        bool C3PhyLoader::ParseKeyframesXKEY(const uint8_t* data, size_t& offset,
            size_t chunkEnd, C3Motion& motion) {
            if (!Read(data, offset, chunkEnd, motion.keyframeCount)) return false;
            motion.keyframes.resize(motion.keyframeCount);

            for (uint32_t i = 0; i < motion.keyframeCount; ++i) {
                auto& kf = motion.keyframes[i];
                uint16_t framePos16;
                if (!Read(data, offset, chunkEnd, framePos16)) return false;
                kf.framePosition = framePos16;

                kf.boneMatrices.resize(motion.boneCount);
                for (uint32_t b = 0; b < motion.boneCount; ++b) {
                    TidyMatrix tidyMat;
                    if (!Read(data, offset, chunkEnd, tidyMat)) return false;
                    kf.boneMatrices[b] = tidyMat.ToMat4();
                }
            }

            YAMEN_CORE_INFO("Loaded {} XKEY keyframes", motion.keyframeCount);
            return true;
        }

        bool C3PhyLoader::ParseKeyframesZKEY(const uint8_t* data, size_t& offset,
            size_t chunkEnd, C3Motion& motion) {
            if (!Read(data, offset, chunkEnd, motion.keyframeCount)) return false;
            motion.keyframes.resize(motion.keyframeCount);

            for (uint32_t i = 0; i < motion.keyframeCount; ++i) {
                auto& kf = motion.keyframes[i];
                uint16_t framePos16;
                if (!Read(data, offset, chunkEnd, framePos16)) return false;
                kf.framePosition = framePos16;

                kf.boneMatrices.resize(motion.boneCount);
                for (uint32_t b = 0; b < motion.boneCount; ++b) {
                    DivInfo divInfo;
                    if (!Read(data, offset, chunkEnd, divInfo)) return false;
                    kf.boneMatrices[b] = divInfo.ToMat4();

                    if (i == 0 && b == 0) {
                        YAMEN_CORE_INFO("ZKEY Bone 0 Frame 0 Quat: [{}, {}, {}, {}] Trans: [{}, {}, {}]",
                            divInfo.quaternion[0], divInfo.quaternion[1], divInfo.quaternion[2],
                            divInfo.quaternion[3], divInfo.translation[0],
                            divInfo.translation[1], divInfo.translation[2]);
                    }
                }
            }

            YAMEN_CORE_INFO("Loaded {} ZKEY keyframes", motion.keyframeCount);
            return true;
        }

        bool C3PhyLoader::ParseKeyframesLegacy(const uint8_t* data, size_t& offset,
            size_t chunkEnd, C3Motion& motion) {
            motion.keyframeCount = motion.frameCount;
            motion.keyframes.resize(motion.keyframeCount);

            for (uint32_t f = 0; f < motion.frameCount; ++f) {
                auto& kf = motion.keyframes[f];
                kf.framePosition = f;
                kf.boneMatrices.resize(motion.boneCount);

                for (uint32_t b = 0; b < motion.boneCount; ++b) {
                    if (!Read(data, offset, chunkEnd, kf.boneMatrices[b])) {
                        YAMEN_CORE_ERROR("Failed to read legacy bone matrix at frame {}, bone {}", f, b);
                        return false;
                    }
                }
            }

            YAMEN_CORE_INFO("Loaded {} legacy keyframes", motion.keyframeCount);
            return true;
        }

        // FIXED: Now takes chunkEnd + correct 76-byte layout
        bool C3PhyLoader::ParsePhysicsChunk(const uint8_t* data, size_t& offset,
            size_t chunkEnd, C3Phy& phy, bool isPhy4) {
            size_t start_offset = offset;
            YAMEN_CORE_INFO("=== ParsePhysicsChunk START ===");
            YAMEN_CORE_INFO("  Start offset: {}, Chunk end: {}", offset, chunkEnd);

            // Read name length
            uint32_t nameLen;
            if (!Read(data, offset, chunkEnd, nameLen)) {
                YAMEN_CORE_ERROR("  Failed to read name length");
                return false;
            }

            std::string name = "unnamed";
            if (nameLen > 0 && nameLen < 256 && offset + nameLen <= chunkEnd) {
                name.assign(reinterpret_cast<const char*>(data + offset), nameLen);
                offset += nameLen;
            }
            YAMEN_CORE_INFO("  Model name: '{}'", name);

            // Read blend count
            uint32_t blendCount = 0;
            if (!Read(data, offset, chunkEnd, blendCount)) return false;
            phy.blendCount = blendCount;
            YAMEN_CORE_INFO("  Blend count: {}", blendCount);

            // Read vertex counts
            if (!Read(data, offset, chunkEnd, phy.normalVertexCount)) return false;
            if (!Read(data, offset, chunkEnd, phy.alphaVertexCount)) return false;

            uint32_t totalVertices = phy.normalVertexCount + phy.alphaVertexCount;
            YAMEN_CORE_INFO("  Vertices: {} normal, {} alpha ({} total)", phy.normalVertexCount, phy.alphaVertexCount, totalVertices);

            if (totalVertices == 0 || totalVertices > 100000) {
                YAMEN_CORE_ERROR("  Invalid vertex count");
                return false;
            }

            size_t baseVertexIndex = phy.vertices.size();
            phy.vertices.resize(baseVertexIndex + totalVertices);

            // === CORRECT PHY4 / zero-blend handling ===
            if (isPhy4 || blendCount == 0) {
                // PHY4 or zero-blend = fixed 40-byte format:
                // pos(12) + normal(12) + uv(8) + padding(8) = 40 bytes
                const uint32_t vertexStride = 40;

                if (offset + totalVertices * vertexStride > chunkEnd) {
                    YAMEN_CORE_ERROR("  Not enough data for {} 40-byte vertices", totalVertices);
                    return false;
                }

                for (uint32_t i = 0; i < totalVertices; ++i) {
                    const uint8_t* vData = data + offset;
                    auto& v = phy.vertices[baseVertexIndex + i];

                    memcpy(&v.position, vData + 0, 12);
                    memcpy(&v.normal, vData + 12, 12);
                    memcpy(&v.texCoord, vData + 24, 8);

                    // No bone data → identity weight
                    v.boneIndices[0] = v.boneIndices[1] = v.boneIndices[2] = v.boneIndices[3] = 0;
                    v.boneWeights[0] = 1.0f;
                    v.boneWeights[1] = v.boneWeights[2] = v.boneWeights[3] = 0.0f;

                    offset += vertexStride;
                }
                YAMEN_CORE_INFO("  Parsed {} 40-byte (PHY4/zero-blend) vertices", totalVertices);
            }
            else if (blendCount > 0 && blendCount <= 4) {
                // Standard blended format: indices + weights + pos + normal
                uint32_t stride = blendCount * 4 + blendCount * 4 + 12 + 12; // indices + weights + pos + norm
                if (offset + totalVertices * stride > chunkEnd) {
                    YAMEN_CORE_ERROR("  Not enough data for blended vertices");
                    return false;
                }

                for (uint32_t i = 0; i < totalVertices; ++i) {
                    const uint8_t* vData = data + offset;
                    auto& v = phy.vertices[baseVertexIndex + i];

                    // Bone indices (uint32_t)
                    for (uint32_t b = 0; b < blendCount; ++b)
                        v.boneIndices[b] = reinterpret_cast<const uint32_t*>(vData)[b];
                    for (uint32_t b = blendCount; b < 4; ++b)
                        v.boneIndices[b] = 0;

                    // Bone weights (float)
                    const float* weights = reinterpret_cast<const float*>(vData + blendCount * 4);
                    for (uint32_t b = 0; b < blendCount; ++b)
                        v.boneWeights[b] = weights[b];
                    for (uint32_t b = blendCount; b < 4; ++b)
                        v.boneWeights[b] = 0.0f;

                    // Position + Normal
                    memcpy(&v.position, vData + blendCount * 8, 12);
                    memcpy(&v.normal, vData + blendCount * 8 + 12, 12);
                    v.texCoord = glm::vec2(0.0f); // No UV in blended format

                    offset += stride;
                }
            }
            else {
                YAMEN_CORE_ERROR("  Unsupported blend count: {}", blendCount);
                return false;
            }

            // === Triangle counts ===
            if (!Read(data, offset, chunkEnd, phy.normalTriCount)) return false;
            if (!Read(data, offset, chunkEnd, phy.alphaTriCount)) return false;

            uint32_t totalTriangles = phy.normalTriCount + phy.alphaTriCount;
            if (totalTriangles > 100000) {
                YAMEN_CORE_ERROR("  INVALID triangle count: {} (likely parsing error)", totalTriangles);
                return false;
            }
            YAMEN_CORE_INFO("  Triangles: {} normal, {} alpha ({} total)", phy.normalTriCount, phy.alphaTriCount, totalTriangles);

            // === Indices ===
            size_t baseIndexIndex = phy.indices.size();
            phy.indices.resize(baseIndexIndex + totalTriangles * 3);

            for (uint32_t i = 0; i < totalTriangles * 3; ++i) {
                uint16_t idx;
                if (!Read(data, offset, chunkEnd, idx)) return false;
                phy.indices[baseIndexIndex + i] = idx + static_cast<uint16_t>(baseVertexIndex);
            }

            // Optional: texture, bbox, etc.
            ReadString(data, offset, chunkEnd, phy.textureName);
            if (!phy.textureName.empty())
                YAMEN_CORE_INFO("  Texture: '{}'", phy.textureName);

            YAMEN_CORE_INFO("=== ParsePhysicsChunk SUCCESS === {} bytes used", offset - start_offset);
            return true;
        }

        void C3PhyLoader::InterpolateBones(const C3Motion& motion, float frame,
            std::vector<glm::mat4>& outMatrices) {
            if (motion.keyframes.empty() || motion.boneCount == 0) {
                outMatrices.clear();
                return;
            }

            outMatrices.resize(motion.boneCount);
            frame = std::max(0.0f, std::min(frame, static_cast<float>(motion.frameCount - 1)));

            size_t kf1 = 0, kf2 = 0;
            float t = 0.0f;

            for (size_t i = 0; i < motion.keyframes.size(); ++i) {
                if (motion.keyframes[i].framePosition <= frame) kf1 = i;
                if (motion.keyframes[i].framePosition >= frame) { kf2 = i; break; }
            }
            if (kf1 < motion.keyframes.size() && kf2 < motion.keyframes.size() && kf1 != kf2) {
                float f1 = static_cast<float>(motion.keyframes[kf1].framePosition);
                float f2 = static_cast<float>(motion.keyframes[kf2].framePosition);
                t = (frame - f1) / (f2 - f1);
            }

            for (uint32_t b = 0; b < motion.boneCount; ++b) {
                const glm::mat4& m1 = motion.keyframes[kf1].boneMatrices[b];
                const glm::mat4& m2 = motion.keyframes[kf2].boneMatrices[b];
                outMatrices[b] = m1 * (1.0f - t) + m2 * t;
            }
        }

        bool C3PhyLoader::ReadString(const uint8_t* data, size_t& offset,
            size_t dataSize, std::string& out) {
            uint32_t length;
            if (!Read(data, offset, dataSize, length)) return false;
            if (length == 0 || length > 1024 || offset + length > dataSize) return false;
            out.assign(reinterpret_cast<const char*>(data + offset), length);
            offset += length;
            return true;
        }

        bool C3PhyLoader::ReadChunkHeader(const uint8_t* data, size_t& offset,
            size_t dataSize, char chunkID[4],
            uint32_t& chunkSize) {
            if (offset + 8 > dataSize) return false;
            memcpy(chunkID, data + offset, 4);
            offset += 4;
            memcpy(&chunkSize, data + offset, 4);
            offset += 4;
            return true;
        }

    } // namespace Assets
} // namespace Yamen