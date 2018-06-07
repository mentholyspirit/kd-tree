#pragma once
#include "triangle.h"
#include "aabb.h"
#include <vector>

enum SAHEventType : char
{
    kEventEnd,
    kEventPlanar,
    kEventStart
};

struct SAHEvent
{
    int tri;
    float planePosition;
    SAHEventType type;
};

 class KDNode
 {
    std::unique_ptr<KDNode> m_Left;
    std::unique_ptr<KDNode> m_Right;
    AABB m_AABB;
    std::vector<Triangle> triangles;
    KDNode();

public:
    static KDNode* CreateNode(const std::vector<Triangle>& faces, const AABB& aabb, const std::vector<SAHEvent>* events, int depth);
    const KDNode* GetLeft() const { return m_Left.get(); }
    const KDNode* GetRight() const { return m_Right.get(); }
    const AABB& GetAABB() const { return m_AABB; }
    const std::vector<Triangle>& GetTriangles() const { return triangles; }
};
