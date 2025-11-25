#include "Graphics/Mesh/MeshBuilder.h"
#include <glm/gtc/constants.hpp>

namespace Yamen::Graphics {

    void MeshBuilder::CreateCube(std::vector<Vertex3D>& vertices, std::vector<uint32_t>& indices, float size) {
        float half = size * 0.5f;

        // 24 vertices (4 per face for proper normals)
        vertices = {
            // Front face (+Z)
            {{-half, -half,  half}, {0, 0, 1}, {0, 0}},
            {{ half, -half,  half}, {0, 0, 1}, {1, 0}},
            {{ half,  half,  half}, {0, 0, 1}, {1, 1}},
            {{-half,  half,  half}, {0, 0, 1}, {0, 1}},

            // Back face (-Z)
            {{ half, -half, -half}, {0, 0, -1}, {0, 0}},
            {{-half, -half, -half}, {0, 0, -1}, {1, 0}},
            {{-half,  half, -half}, {0, 0, -1}, {1, 1}},
            {{ half,  half, -half}, {0, 0, -1}, {0, 1}},

            // Top face (+Y)
            {{-half,  half,  half}, {0, 1, 0}, {0, 0}},
            {{ half,  half,  half}, {0, 1, 0}, {1, 0}},
            {{ half,  half, -half}, {0, 1, 0}, {1, 1}},
            {{-half,  half, -half}, {0, 1, 0}, {0, 1}},

            // Bottom face (-Y)
            {{-half, -half, -half}, {0, -1, 0}, {0, 0}},
            {{ half, -half, -half}, {0, -1, 0}, {1, 0}},
            {{ half, -half,  half}, {0, -1, 0}, {1, 1}},
            {{-half, -half,  half}, {0, -1, 0}, {0, 1}},

            // Right face (+X)
            {{ half, -half,  half}, {1, 0, 0}, {0, 0}},
            {{ half, -half, -half}, {1, 0, 0}, {1, 0}},
            {{ half,  half, -half}, {1, 0, 0}, {1, 1}},
            {{ half,  half,  half}, {1, 0, 0}, {0, 1}},

            // Left face (-X)
            {{-half, -half, -half}, {-1, 0, 0}, {0, 0}},
            {{-half, -half,  half}, {-1, 0, 0}, {1, 0}},
            {{-half,  half,  half}, {-1, 0, 0}, {1, 1}},
            {{-half,  half, -half}, {-1, 0, 0}, {0, 1}}
        };

        // 36 indices (6 faces * 2 triangles * 3 vertices)
        indices = {
            0, 1, 2, 2, 3, 0,       // Front
            4, 5, 6, 6, 7, 4,       // Back
            8, 9, 10, 10, 11, 8,    // Top
            12, 13, 14, 14, 15, 12, // Bottom
            16, 17, 18, 18, 19, 16, // Right
            20, 21, 22, 22, 23, 20  // Left
        };
    }

    void MeshBuilder::CreateSphere(
        std::vector<Vertex3D>& vertices,
        std::vector<uint32_t>& indices,
        float radius,
        uint32_t segments,
        uint32_t rings)
    {
        vertices.clear();
        indices.clear();

        // Generate vertices
        for (uint32_t ring = 0; ring <= rings; ++ring) {
            float phi = glm::pi<float>() * static_cast<float>(ring) / static_cast<float>(rings);
            float y = radius * cos(phi);
            float ringRadius = radius * sin(phi);

            for (uint32_t segment = 0; segment <= segments; ++segment) {
                float theta = 2.0f * glm::pi<float>() * static_cast<float>(segment) / static_cast<float>(segments);
                float x = ringRadius * cos(theta);
                float z = ringRadius * sin(theta);

                glm::vec3 position(x, y, z);
                glm::vec3 normal = glm::normalize(position);
                glm::vec2 texCoord(
                    static_cast<float>(segment) / static_cast<float>(segments),
                    static_cast<float>(ring) / static_cast<float>(rings)
                );

                vertices.emplace_back(position, normal, texCoord);
            }
        }

        // Generate indices
        for (uint32_t ring = 0; ring < rings; ++ring) {
            for (uint32_t segment = 0; segment < segments; ++segment) {
                uint32_t current = ring * (segments + 1) + segment;
                uint32_t next = current + segments + 1;

                indices.push_back(current);
                indices.push_back(next);
                indices.push_back(current + 1);

                indices.push_back(current + 1);
                indices.push_back(next);
                indices.push_back(next + 1);
            }
        }
    }

    void MeshBuilder::CreatePlane(
        std::vector<Vertex3D>& vertices,
        std::vector<uint32_t>& indices,
        float width,
        float height,
        uint32_t subdivisionsX,
        uint32_t subdivisionsY)
    {
        vertices.clear();
        indices.clear();

        float halfWidth = width * 0.5f;
        float halfHeight = height * 0.5f;

        // Generate vertices
        for (uint32_t y = 0; y <= subdivisionsY; ++y) {
            for (uint32_t x = 0; x <= subdivisionsX; ++x) {
                float u = static_cast<float>(x) / static_cast<float>(subdivisionsX);
                float v = static_cast<float>(y) / static_cast<float>(subdivisionsY);

                glm::vec3 position(
                    -halfWidth + u * width,
                    0.0f,
                    -halfHeight + v * height
                );
                glm::vec3 normal(0.0f, 1.0f, 0.0f);
                glm::vec2 texCoord(u, v);

                vertices.emplace_back(position, normal, texCoord);
            }
        }

        // Generate indices
        for (uint32_t y = 0; y < subdivisionsY; ++y) {
            for (uint32_t x = 0; x < subdivisionsX; ++x) {
                uint32_t topLeft = y * (subdivisionsX + 1) + x;
                uint32_t topRight = topLeft + 1;
                uint32_t bottomLeft = topLeft + (subdivisionsX + 1);
                uint32_t bottomRight = bottomLeft + 1;

                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }
    }

    void MeshBuilder::CalculateTangents(std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices) {
        // Calculate tangents for normal mapping
        for (size_t i = 0; i < indices.size(); i += 3) {
            Vertex3D& v0 = vertices[indices[i]];
            Vertex3D& v1 = vertices[indices[i + 1]];
            Vertex3D& v2 = vertices[indices[i + 2]];

            glm::vec3 edge1 = v1.position - v0.position;
            glm::vec3 edge2 = v2.position - v0.position;

            glm::vec2 deltaUV1 = v1.texCoord - v0.texCoord;
            glm::vec2 deltaUV2 = v2.texCoord - v0.texCoord;

            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

            glm::vec3 tangent;
            tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
            tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
            tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
            tangent = glm::normalize(tangent);

            v0.tangent += tangent;
            v1.tangent += tangent;
            v2.tangent += tangent;
        }

        // Normalize tangents
        for (auto& vertex : vertices) {
            vertex.tangent = glm::normalize(vertex.tangent);
        }
    }

} // namespace Yamen::Graphics
