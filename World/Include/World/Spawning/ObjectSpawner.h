#pragma once

#include <Core/Math/Math.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>


namespace Yamen::World {

using namespace Yamen::Core;

// Forward declaration of Entity or GameObject class
// For now, we'll use a placeholder or ID
using EntityID = uint32_t;

class ObjectSpawner {
public:
  using SpawnFunction = std::function<EntityID(const vec3 &)>;

  ObjectSpawner() = default;

  // Register a spawn function for a specific type
  void RegisterType(const std::string &typeName, SpawnFunction spawnFunc);

  // Spawn an object of a specific type
  EntityID Spawn(const std::string &typeName, const vec3 &position);

private:
  std::unordered_map<std::string, SpawnFunction> m_SpawnFunctions;
};

} // namespace Yamen::World
