#include "ECS/Physics/SpatialHash.h"
#include <algorithm>
#include <cmath>

namespace Yamen::ECS {

using namespace Yamen::Core;

SpatialHash::SpatialHash(float cellSize) : m_CellSize(cellSize) {}

void SpatialHash::Clear() {
  m_Grid.clear();
  m_TotalEntries = 0;
}

void SpatialHash::Insert(entt::entity entity, const vec3 &min,
                         const vec3 &max) {
  CellKey minKey, maxKey;
  GetCellRange(min, max, minKey, maxKey);

  // Insert entity into all cells it overlaps
  for (int x = minKey.x; x <= maxKey.x; ++x) {
    for (int y = minKey.y; y <= maxKey.y; ++y) {
      for (int z = minKey.z; z <= maxKey.z; ++z) {
        CellKey key{x, y, z};
        m_Grid[key].push_back(entity);
        m_TotalEntries++;
      }
    }
  }
}

void SpatialHash::Query(const vec3 &min, const vec3 &max,
                        std::vector<entt::entity> &results) const {
  CellKey minKey, maxKey;
  GetCellRange(min, max, minKey, maxKey);

  results.clear();

  // Query all cells in range
  for (int x = minKey.x; x <= maxKey.x; ++x) {
    for (int y = minKey.y; y <= maxKey.y; ++y) {
      for (int z = minKey.z; z <= maxKey.z; ++z) {
        CellKey key{x, y, z};
        auto it = m_Grid.find(key);
        if (it != m_Grid.end()) {
          results.insert(results.end(), it->second.begin(), it->second.end());
        }
      }
    }
  }

  // Remove duplicates (entity might be in multiple cells)
  std::sort(results.begin(), results.end());
  results.erase(std::unique(results.begin(), results.end()), results.end());
}

SpatialHash::CellKey SpatialHash::GetCellKey(const vec3 &position) const {
  return CellKey{static_cast<int>(std::floor(position.x / m_CellSize)),
                 static_cast<int>(std::floor(position.y / m_CellSize)),
                 static_cast<int>(std::floor(position.z / m_CellSize))};
}

void SpatialHash::GetCellRange(const vec3 &min, const vec3 &max,
                               CellKey &minKey, CellKey &maxKey) const {
  minKey = GetCellKey(min);
  maxKey = GetCellKey(max);
}

} // namespace Yamen::ECS
