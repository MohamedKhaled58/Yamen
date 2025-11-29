#include "World/Spatial/QuadTree.h"
#include "World/Culling/Frustum.h"

namespace Yamen::World {

QuadTree::QuadTree(const Core::AABB &bounds, int capacity, int maxDepth)
    : m_Bounds(bounds), m_Capacity(capacity), m_MaxDepth(maxDepth), m_Depth(0) {
  m_Objects.reserve(capacity);
}

// In CreateChild:
static QuadTree *CreateChild(const Core::AABB &bounds, int capacity,
                             int maxDepth, int depth) {
  auto *qt = new QuadTree(bounds, capacity, maxDepth);
  qt->SetDepth(depth);
  return qt;
}

QuadTree::~QuadTree() = default;

bool QuadTree::Insert(entt::entity entity, const Core::AABB &bounds) {
  if (!m_Bounds.Intersects(bounds)) {
    return false;
  }

  if (m_Objects.size() < m_Capacity || m_Depth >= m_MaxDepth) {
    m_Objects.push_back({entity, bounds});
    return true;
  }

  if (!m_Divided) {
    Subdivide();
  }

  // Try to insert into children
  bool inserted = false;
  for (int i = 0; i < 4; ++i) {
    if (m_Children[i]->Insert(entity, bounds)) {
      inserted = true;
      // Don't break, object might span multiple children
      // Actually, standard quadtree usually stores in the node that fully
      // contains it OR stores in all intersecting nodes (reference). For
      // simplicity here, we store in all intersecting nodes.
    }
  }

  return inserted;
}

bool QuadTree::Remove(entt::entity entity, const Core::AABB &bounds) {
  if (!m_Bounds.Intersects(bounds)) {
    return false;
  }

  bool removed = false;

  // Check this node
  auto it = std::remove_if(
      m_Objects.begin(), m_Objects.end(),
      [entity](const QuadTreeData &data) { return data.entity == entity; });

  if (it != m_Objects.end()) {
    m_Objects.erase(it, m_Objects.end());
    removed = true;
  }

  if (m_Divided) {
    for (int i = 0; i < 4; ++i) {
      if (m_Children[i]->Remove(entity, bounds)) {
        removed = true;
      }
    }
  }

  return removed;
}

void QuadTree::Query(const Core::AABB &range,
                     std::vector<entt::entity> &found) const {
  if (!m_Bounds.Intersects(range)) {
    return;
  }

  for (const auto &obj : m_Objects) {
    if (range.Intersects(obj.bounds)) {
      found.push_back(obj.entity);
    }
  }

  if (m_Divided) {
    for (int i = 0; i < 4; ++i) {
      m_Children[i]->Query(range, found);
    }
  }
}

void QuadTree::Query(const Frustum &frustum,
                     std::vector<entt::entity> &found) const {
  if (!frustum.ContainsBox(m_Bounds.Min, m_Bounds.Max)) {
    return;
  }

  for (const auto &obj : m_Objects) {
    if (frustum.ContainsBox(obj.bounds.Min, obj.bounds.Max)) {
      found.push_back(obj.entity);
    }
  }

  if (m_Divided) {
    for (int i = 0; i < 4; ++i) {
      m_Children[i]->Query(frustum, found);
    }
  }
}

void QuadTree::Clear() {
  m_Objects.clear();
  if (m_Divided) {
    for (int i = 0; i < 4; ++i) {
      m_Children[i].reset();
    }
    m_Divided = false;
  }
}

void QuadTree::Subdivide() {
  Core::vec3 min = m_Bounds.Min;
  Core::vec3 max = m_Bounds.Max;
  Core::vec3 center = m_Bounds.GetCenter();

  // NW (Top Left) - assuming Z is up or Y is up?
  // Let's assume standard 3D game: Y is up, X/Z are ground.
  // QuadTree usually splits X/Z plane.

  // NW: min.x, center.z -> center.x, max.z
  Core::AABB nwBounds(Core::vec3(min.x, min.y, center.z),
                      Core::vec3(center.x, max.y, max.z));

  // NE: center.x, center.z -> max.x, max.z
  Core::AABB neBounds(Core::vec3(center.x, min.y, center.z),
                      Core::vec3(max.x, max.y, max.z));

  // SW: min.x, min.z -> center.x, center.z
  Core::AABB swBounds(Core::vec3(min.x, min.y, min.z),
                      Core::vec3(center.x, max.y, center.z));

  // SE: center.x, min.z -> max.x, center.z
  Core::AABB seBounds(Core::vec3(center.x, min.y, min.z),
                      Core::vec3(max.x, max.y, center.z));

  m_Children[0].reset(
      CreateChild(nwBounds, m_Capacity, m_MaxDepth, m_Depth + 1));
  m_Children[1].reset(
      CreateChild(neBounds, m_Capacity, m_MaxDepth, m_Depth + 1));
  m_Children[2].reset(
      CreateChild(swBounds, m_Capacity, m_MaxDepth, m_Depth + 1));
  m_Children[3].reset(
      CreateChild(seBounds, m_Capacity, m_MaxDepth, m_Depth + 1));

  m_Divided = true;
}

} // namespace Yamen::World
