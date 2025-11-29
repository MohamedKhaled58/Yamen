#include "World/Spawning/ObjectSpawner.h"
#include <Core/Logging/Logger.h>

namespace Yamen::World {

using namespace Yamen::Core;

void ObjectSpawner::RegisterType(const std::string &typeName,
                                 SpawnFunction spawnFunc) {
  m_SpawnFunctions[typeName] = spawnFunc;
  YAMEN_CORE_INFO("Registered spawn type: {0}", typeName);
}

EntityID ObjectSpawner::Spawn(const std::string &typeName,
                              const vec3 &position) {
  auto it = m_SpawnFunctions.find(typeName);
  if (it != m_SpawnFunctions.end()) {
    YAMEN_CORE_TRACE("Spawning object of type {0} at ({1}, {2}, {3})", typeName,
                     position.x, position.y, position.z);
    return it->second(position);
  }

  YAMEN_CORE_WARN("Failed to spawn object: Unknown type '{0}'", typeName);
  return 0; // Return invalid ID
}

} // namespace Yamen::World
