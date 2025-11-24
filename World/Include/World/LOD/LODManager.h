#pragma once

#include "Core/Math/Math.h"
#include <vector>

namespace Yamen::World {

    class LODManager {
    public:
        struct LODLevel {
            float distance; // Distance at which this LOD becomes active (or stops being active)
            int level;      // 0 = highest detail
        };

        LODManager();
        
        void AddLODLevel(float distance, int level);
        
        int GetLODLevel(const Core::vec3& objectPos, const Core::vec3& viewerPos) const;
        int GetLODLevel(float distanceSquared) const;

    private:
        std::vector<LODLevel> m_Levels;
    };

} // namespace Yamen::World
