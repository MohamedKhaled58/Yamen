#include "Graphics/Mesh/MeshLoader.h"
#include <Core/Logging/Logger.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>

namespace Yamen::Graphics {

    std::unique_ptr<Mesh> MeshLoader::LoadOBJ(GraphicsDevice& device, const std::string& filepath) {
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> texCoords;
        std::vector<Vertex3D> vertices;
        std::vector<uint32_t> indices;

        std::ifstream file(filepath);
        if (!file.is_open()) {
            YAMEN_CORE_ERROR("Failed to open OBJ file: {0}", filepath);
            return nullptr;
        }

        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string prefix;
            ss >> prefix;

            if (prefix == "v") {
                glm::vec3 pos;
                ss >> pos.x >> pos.y >> pos.z;
                positions.push_back(pos);
            } else if (prefix == "vt") {
                glm::vec2 uv;
                ss >> uv.x >> uv.y;
                texCoords.push_back(uv);
            } else if (prefix == "vn") {
                glm::vec3 norm;
                ss >> norm.x >> norm.y >> norm.z;
                normals.push_back(norm);
            } else if (prefix == "f") {
                // Parse face (v/vt/vn)
                // Triangulate if necessary (assuming triangles for now)
                for (int i = 0; i < 3; ++i) {
                    std::string vertexStr;
                    ss >> vertexStr;

                    // Parse indices
                    std::stringstream vs(vertexStr);
                    std::string segment;
                    std::vector<std::string> segments;
                    while (std::getline(vs, segment, '/')) {
                        segments.push_back(segment);
                    }

                    Vertex3D vertex;
                    
                    // Position
                    int posIdx = std::stoi(segments[0]) - 1;
                    vertex.position = positions[posIdx];

                    // TexCoord
                    if (segments.size() > 1 && !segments[1].empty()) {
                        int uvIdx = std::stoi(segments[1]) - 1;
                        vertex.texCoord = texCoords[uvIdx];
                    }

                    // Normal
                    if (segments.size() > 2 && !segments[2].empty()) {
                        int normIdx = std::stoi(segments[2]) - 1;
                        vertex.normal = normals[normIdx];
                    }
                    
                    //TODO Check ??
                    vertex.tangent = glm::vec4(1.0f); // Default white

                    vertices.push_back(vertex);
                    indices.push_back(static_cast<uint32_t>(indices.size()));
                }
            }
        }

        auto mesh = std::make_unique<Mesh>(device);
        mesh->Create(vertices, indices);
        
        YAMEN_CORE_INFO("Loaded OBJ mesh: {0} (Vertices: {1}, Indices: {2})", filepath, vertices.size(), indices.size());
        return mesh;
    }

} // namespace Yamen::Graphics
