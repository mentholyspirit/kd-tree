#include "kdtree.h"
#include <chrono>

static void CreateEventList(const std::vector<Triangle>& triangles, std::vector<SAHEvent>* events)
{
    //create "events" for SAH in each dimension and sort them to effectively sweep when finding splits
    for (char k = 0; k < kAxesCount; k++)
    {
        events[k].reserve(triangles.size() * 2);
        for (int i = 0; i < triangles.size(); i++)
        {
            const Triangle& tri = triangles[i];
            if (fabsf(tri.GetNormal()[k]) == 1.0)
            {
                SAHEvent ev0;
                ev0.tri = i;
                ev0.planePosition = tri.GetAxisMin((Axis)k);
                ev0.type = kEventPlanar;
                events[k].push_back(ev0);
            }
            else
            {
                SAHEvent ev0, ev1;
                ev0.tri = i;
                ev0.planePosition = tri.GetAxisMin((Axis)k);
                ev1.planePosition = tri.GetAxisMax((Axis)k);
                ev1.tri = i;
                ev0.type = kEventStart;
                ev1.type = kEventEnd;
                events[k].push_back(ev0);
                events[k].push_back(ev1);
            }
        }
        std::sort(events[k].begin(), events[k].end(), EventSortPredicate);
    }
}

KDTree::KDTree(const std::vector<Triangle>& faces, const AABB& aabb)
{
    std::vector<SAHEvent> events[kAxesCount];
    CreateEventList(faces, events);
    KDNode* node = KDNode::CreateNode(faces, aabb, events, 0);
    if (node != nullptr)
        m_RootNode.reset(node);
}
