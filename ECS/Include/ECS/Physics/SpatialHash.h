#pragma once

#include <Core/Math/Math.h>
#include <entt/entt.hpp>
#include <functional>
#include <unordered_map>
#include <vector>


namespace Yamen::ECS {

/**
 * @brief Spatial Hash Grid for efficient broad-phase collision detection
 *
 * Reduces collision detection from O(N²) to O(N) by partitioning space into
 * cells. Only objects in the same or neighboring cells need to be tested for
 * collision.
 */
class SpatialHash {
public:
  SpatialHash(float cellSize = 2.0f);

  // Clear all entries
  void Clear();

  // Insert an entity with its AABB
  void Insert(entt::entity entity, const Yamen::Core::vec3 &min,
              const Yamen::Core::vec3 &max);

  // Query entities that could collide with the given AABB
  void Query(const Yamen::Core::vec3 &min, const Yamen::Core::vec3 &max,
             std::vector<entt::entity> &results) const;

  // Get cell size
  float GetCellSize() const { return m_CellSize; }

  // Statistics
  int GetCellCount() const { return static_cast<int>(m_Grid.size()); }
  int GetTotalEntries() const { return m_TotalEntries; }

private:
  struct CellKey {
    int x, y, z;

    bool operator==(const CellKey &other) const {
      return x == other.x && y == other.y && z == other.z;
    }
  };

  struct CellKeyHash {
    std::size_t operator()(const CellKey &key) const {
      // Simple hash combining
      return std::hash<int>()(key.x) ^ (std::hash<int>()(key.y) << 1) ^
             (std::hash<int>()(key.z) << 2);
    }
  };

  CellKey GetCellKey(const Yamen::Core::vec3 &position) const;
  void GetCellRange(const Yamen::Core::vec3 &min, const Yamen::Core::vec3 &max,
                    CellKey &minKey, CellKey &maxKey) const;

  float m_CellSize;
  std::unordered_map<CellKey, std::vector<entt::entity>, CellKeyHash> m_Grid;
  int m_TotalEntries = 0;
};

} // namespace Yamen::ECS
