#include "World/Culling/Frustum.h"

namespace Yamen::World {

using namespace Yamen::Core;

Plane::Plane(const vec3 &p1, const vec3 &p2, const vec3 &p3) {
  Normal = Math::Normalize(Math::Cross(p2 - p1, p3 - p1));
  Distance = -Math::Dot(Normal, p1);
}

float Plane::GetSignedDistance(const vec3 &point) const {
  return Math::Dot(Normal, point) + Distance;
}

void Frustum::Update(const mat4 &viewProj) {
  // Extract planes from View-Projection matrix
  // Gribb/Hartmann method
  // Note: viewProj is Row-Major (DirectXMath), so we access as [row][col]
  // We transpose the indices relative to the original GLM code

  // Left plane
  m_Planes[LEFT].Normal.x = viewProj[3][0] + viewProj[0][0];
  m_Planes[LEFT].Normal.y = viewProj[3][1] + viewProj[0][1];
  m_Planes[LEFT].Normal.z = viewProj[3][2] + viewProj[0][2];
  m_Planes[LEFT].Distance = viewProj[3][3] + viewProj[0][3];

  // Right plane
  m_Planes[RIGHT].Normal.x = viewProj[3][0] - viewProj[0][0];
  m_Planes[RIGHT].Normal.y = viewProj[3][1] - viewProj[0][1];
  m_Planes[RIGHT].Normal.z = viewProj[3][2] - viewProj[0][2];
  m_Planes[RIGHT].Distance = viewProj[3][3] - viewProj[0][3];

  // Bottom plane
  m_Planes[BOTTOM].Normal.x = viewProj[3][0] + viewProj[1][0];
  m_Planes[BOTTOM].Normal.y = viewProj[3][1] + viewProj[1][1];
  m_Planes[BOTTOM].Normal.z = viewProj[3][2] + viewProj[1][2];
  m_Planes[BOTTOM].Distance = viewProj[3][3] + viewProj[1][3];

  // Top plane
  m_Planes[TOP].Normal.x = viewProj[3][0] - viewProj[1][0];
  m_Planes[TOP].Normal.y = viewProj[3][1] - viewProj[1][1];
  m_Planes[TOP].Normal.z = viewProj[3][2] - viewProj[1][2];
  m_Planes[TOP].Distance = viewProj[3][3] - viewProj[1][3];

  // Near plane
  m_Planes[NEAR_PLANE].Normal.x = viewProj[3][0] + viewProj[2][0];
  m_Planes[NEAR_PLANE].Normal.y = viewProj[3][1] + viewProj[2][1];
  m_Planes[NEAR_PLANE].Normal.z = viewProj[3][2] + viewProj[2][2];
  m_Planes[NEAR_PLANE].Distance = viewProj[3][3] + viewProj[2][3];

  // Far plane
  m_Planes[FAR_PLANE].Normal.x = viewProj[3][0] - viewProj[2][0];
  m_Planes[FAR_PLANE].Normal.y = viewProj[3][1] - viewProj[2][1];
  m_Planes[FAR_PLANE].Normal.z = viewProj[3][2] - viewProj[2][2];
  m_Planes[FAR_PLANE].Distance = viewProj[3][3] - viewProj[2][3];

  // Normalize planes
  for (auto &plane : m_Planes) {
    float length = Math::Length(plane.Normal);
    plane.Normal /= length;
    plane.Distance /= length;
  }
}

bool Frustum::ContainsPoint(const vec3 &point) const {
  for (const auto &plane : m_Planes) {
    if (plane.GetSignedDistance(point) < 0) {
      return false;
    }
  }
  return true;
}

bool Frustum::ContainsSphere(const vec3 &center, float radius) const {
  for (const auto &plane : m_Planes) {
    if (plane.GetSignedDistance(center) < -radius) {
      return false;
    }
  }
  return true;
}

bool Frustum::ContainsBox(const vec3 &min, const vec3 &max) const {
  // Check if any point of the box is inside all planes
  // Optimization: check positive vertex against plane

  for (const auto &plane : m_Planes) {
    vec3 positiveVertex = min;
    if (plane.Normal.x >= 0)
      positiveVertex.x = max.x;
    if (plane.Normal.y >= 0)
      positiveVertex.y = max.y;
    if (plane.Normal.z >= 0)
      positiveVertex.z = max.z;

    if (plane.GetSignedDistance(positiveVertex) < 0) {
      return false;
    }
  }
  return true;
}

} // namespace Yamen::World
