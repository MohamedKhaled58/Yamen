#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace Yamen::World {

    struct EntityData {
        uint32_t ID;
        std::string Name;
        glm::vec3 Position;
        glm::vec3 Rotation;
        glm::vec3 Scale;
        // Add more components as needed
    };

    class WorldSerializer {
    public:
        WorldSerializer() = default;

        // Save world to file
        bool SaveWorld(const std::string& filepath, const std::vector<EntityData>& entities);

        // Load world from file
        bool LoadWorld(const std::string& filepath, std::vector<EntityData>& outEntities);

    private:
        // Helper methods for serialization formats (e.g., JSON, Binary)
        // For now, we'll use a simple text format or binary
    };

} // namespace Yamen::World
