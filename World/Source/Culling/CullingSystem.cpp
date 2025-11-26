#include "World/Culling/CullingSystem.h"
#include <algorithm>

namespace Yamen::World {

    void CullingSystem::Update(const glm::mat4& viewProj) {
        m_Frustum.Update(viewProj);
    }

    void CullingSystem::RegisterObject(uint32_t entityID, const glm::vec3& pos, float radius) {
        CullableObject obj;
        obj.EntityID = entityID;
        obj.Position = pos;
        obj.Radius = radius;
        obj.IsVisible = true;
        // Set box to invalid for sphere-only objects, or approximate
        obj.BoxMin = pos - glm::vec3(radius);
        obj.BoxMax = pos + glm::vec3(radius);
        
        m_Objects.push_back(obj);
    }

    void CullingSystem::RegisterObject(uint32_t entityID, const glm::vec3& min, const glm::vec3& max) {
        CullableObject obj;
        obj.EntityID = entityID;
        obj.BoxMin = min;
        obj.BoxMax = max;
        obj.Position = (min + max) * 0.5f;
        obj.Radius = glm::length(max - min) * 0.5f;
        obj.IsVisible = true;

        m_Objects.push_back(obj);
    }

    void CullingSystem::UnregisterObject(uint32_t entityID) {
        m_Objects.erase(
            std::remove_if(m_Objects.begin(), m_Objects.end(), 
                [entityID](const CullableObject& obj) { return obj.EntityID == entityID; }),
            m_Objects.end());
    }

    void CullingSystem::Cull() {
        m_VisibleEntities.clear();
        m_VisibleEntities.reserve(m_Objects.size());

        for (auto& obj : m_Objects) {
            // Use sphere culling for now as it's faster
            if (m_Frustum.ContainsSphere(obj.Position, obj.Radius)) {
                obj.IsVisible = true;
                m_VisibleEntities.push_back(obj.EntityID);
            } else {
                obj.IsVisible = false;
            }
        }
    }

    const std::vector<uint32_t>& CullingSystem::GetVisibleEntities() const {
        return m_VisibleEntities;
    }

} // namespace Yamen::World
