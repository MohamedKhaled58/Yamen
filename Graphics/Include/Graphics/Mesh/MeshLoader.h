#pragma once

#include "Graphics/Mesh/Mesh.h"
#include <string>
#include <memory>

namespace Yamen::Graphics {

    class MeshLoader {
    public:
        // Load mesh from OBJ file
        static std::unique_ptr<Mesh> LoadOBJ(GraphicsDevice& device, const std::string& filepath);

    private:
        // Helper to parse OBJ line
        static void ParseOBJLine(const std::string& line);
    };

} // namespace Yamen::Graphics
