#pragma once

#include "triangle.h"
#include "kdnode.h"
#include "aabb.h"
#include <vector>

class KDTree
{
    std::unique_ptr<KDNode> m_RootNode;
public:
    KDTree(const std::vector<Triangle>& faces, const AABB& aabb);
    const KDNode* GetRoot() const
    {
        return m_RootNode.get();
    }
};

inline bool EventSortPredicate(const SAHEvent& ev0, const SAHEvent& ev1)
{
    return (ev0.planePosition == ev1.planePosition) ? ev0.type < ev1.type : ev0.planePosition < ev1.planePosition;
}
