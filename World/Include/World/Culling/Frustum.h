#pragma once

#include <Core/Math/Math.h>
#include <array>

namespace Yamen::World {

using namespace Yamen::Core;

struct Plane {
  vec3 Normal;
  float Distance;

  Plane() = default;
  Plane(const vec3 &p1, const vec3 &p2, const vec3 &p3);

  float GetSignedDistance(const vec3 &point) const;
};

class Frustum {
public:
  Frustum() = default;

  // Update frustum from view-projection matrix
  void Update(const mat4 &viewProj);

  // Intersection tests
  bool ContainsPoint(const vec3 &point) const;
  bool ContainsSphere(const vec3 &center, float radius) const;
  bool ContainsBox(const vec3 &min, const vec3 &max) const;

private:
  std::array<Plane, 6> m_Planes;

  enum { LEFT = 0, RIGHT, BOTTOM, TOP, NEAR_PLANE, FAR_PLANE };
};

} // namespace Yamen::World
