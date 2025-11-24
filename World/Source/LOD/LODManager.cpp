#include "World/LOD/LODManager.h"
#include <algorithm>

namespace Yamen::World {

    LODManager::LODManager() {
        // Default levels if none added
        m_Levels = {
            { 50.0f, 0 },
            { 100.0f, 1 },
            { 200.0f, 2 }
        };
    }

    void LODManager::AddLODLevel(float distance, int level) {
        m_Levels.push_back({ distance, level });
        // Sort by distance
        std::sort(m_Levels.begin(), m_Levels.end(), 
            [](const LODLevel& a, const LODLevel& b) { return a.distance < b.distance; });
    }

    int LODManager::GetLODLevel(const Core::vec3& objectPos, const Core::vec3& viewerPos) const {
        float distSq = glm::distance2(objectPos, viewerPos);
        return GetLODLevel(distSq);
    }

    int LODManager::GetLODLevel(float distanceSquared) const {
        float dist = std::sqrt(distanceSquared);
        
        for (const auto& level : m_Levels) {
            if (dist < level.distance) {
                return level.level;
            }
        }
        
        // Return lowest detail if beyond all ranges
        return m_Levels.empty() ? 0 : m_Levels.back().level + 1;
    }

} // namespace Yamen::World
