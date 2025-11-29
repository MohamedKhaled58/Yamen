#pragma once

#include <Core/Math/Math.h>
#include <string>
#include <vector>


namespace Yamen::World {

using namespace Yamen::Core;

struct EntityData {
  uint32_t ID;
  std::string Name;
  vec3 Position;
  vec3 Rotation;
  vec3 Scale;
  // Add more components as needed
};

class WorldSerializer {
public:
  WorldSerializer() = default;

  // Save world to file
  bool SaveWorld(const std::string &filepath,
                 const std::vector<EntityData> &entities);

  // Load world from file
  bool LoadWorld(const std::string &filepath,
                 std::vector<EntityData> &outEntities);

private:
  // Helper methods for serialization formats (e.g., JSON, Binary)
  // For now, we'll use a simple text format or binary
};

} // namespace Yamen::World
