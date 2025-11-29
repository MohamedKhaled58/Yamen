#pragma once

#include "World/Culling/Frustum.h"
#include <Core/Math/Math.h>
#include <vector>


namespace Yamen::World {

using namespace Yamen::Core;

struct CullableObject {
  uint32_t EntityID;
  vec3 Position;
  float Radius; // For sphere culling
  vec3 BoxMin;  // For AABB culling
  vec3 BoxMax;
  bool IsVisible;
};

class CullingSystem {
public:
  CullingSystem() = default;

  void Update(const mat4 &viewProj);

  // Add object to be culled
  void RegisterObject(uint32_t entityID, const vec3 &pos, float radius);
  void RegisterObject(uint32_t entityID, const vec3 &min, const vec3 &max);

  // Remove object
  void UnregisterObject(uint32_t entityID);

  // Perform culling
  void Cull();

  // Get visible entities
  const std::vector<uint32_t> &GetVisibleEntities() const;

private:
  Frustum m_Frustum;
  std::vector<CullableObject> m_Objects;
  std::vector<uint32_t> m_VisibleEntities;
};

} // namespace Yamen::World
