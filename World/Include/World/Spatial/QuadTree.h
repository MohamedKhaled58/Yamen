#pragma once

#include "Core/Math/Math.h"
#include <vector>
#include <memory>
#include <entt/entt.hpp>

namespace Yamen::World {

    struct QuadTreeData {
        entt::entity entity;
        Core::AABB bounds;
    };

    class QuadTree {
    public:
        QuadTree(const Core::AABB& bounds, int capacity = 4, int maxDepth = 5);
        ~QuadTree();

        /**
         * @brief Insert an entity into the tree
         */
        bool Insert(entt::entity entity, const Core::AABB& bounds);

        /**
         * @brief Remove an entity from the tree
         * Note: This is expensive as it requires searching. 
         * Better to clear and rebuild for dynamic objects or use a loose quadtree.
         */
        bool Remove(entt::entity entity, const Core::AABB& bounds);

        /**
         * @brief Query objects within a range
         */
        void Query(const Core::AABB& range, std::vector<entt::entity>& found) const;
        
        /**
         * @brief Query objects within a frustum
         */
        void Query(const Core::Frustum& frustum, std::vector<entt::entity>& found) const;

        /**
         * @brief Clear the tree
         */
        void Clear();

        int GetDepth() const noexcept { return m_Depth; }
        void SetDepth(int depth) noexcept { m_Depth = depth; }



    private:
        void Subdivide();

        Core::AABB m_Bounds;
        int m_Capacity;
        int m_MaxDepth;
        int m_Depth;
        
        std::vector<QuadTreeData> m_Objects;
        std::unique_ptr<QuadTree> m_Children[4]; // NW, NE, SW, SE
        bool m_Divided = false;
    };

} // namespace Yamen::World
