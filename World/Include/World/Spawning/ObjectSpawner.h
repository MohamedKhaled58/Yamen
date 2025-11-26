#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>

namespace Yamen::World {

    // Forward declaration of Entity or GameObject class
    // For now, we'll use a placeholder or ID
    using EntityID = uint32_t;

    class ObjectSpawner {
    public:
        using SpawnFunction = std::function<EntityID(const glm::vec3&)>;

        ObjectSpawner() = default;

        // Register a spawn function for a specific type
        void RegisterType(const std::string& typeName, SpawnFunction spawnFunc);

        // Spawn an object of a specific type
        EntityID Spawn(const std::string& typeName, const glm::vec3& position);

    private:
        std::unordered_map<std::string, SpawnFunction> m_SpawnFunctions;
    };

} // namespace Yamen::World
