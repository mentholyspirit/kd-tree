#include "kdnode.h"
#include <algorithm>
#include <limits>
#include "kdtree.h"

KDNode::KDNode() {}

enum SplitSide : uint8_t
{
    kSplitSideLeft,
    kSplitSideRight,
    kSplitSideBoth
};

//These values are estimations of traversal and intersection cost, λ bias is to reward non-flat empty nodes
#define LAMBDA_BIAS 0.8f
#define K_TRAVERSAL 1.0f
#define K_INTERSECTION 0.1f

inline float CalculateSurfaceArea(const AABB& aabb)
{
    float width = aabb.max[kAxisX] - aabb.min[kAxisX];
    float height = aabb.max[kAxisY] - aabb.min[kAxisY];
    float depth = aabb.max[kAxisZ] - aabb.min[kAxisZ];
    return (width * height + depth * height + depth * width) * 2;
}

inline void SplitBox(const AABB& aabb, float pos, uint8_t axis, __restrict AABB* left, __restrict AABB* right)
{
    *left = aabb;
    *right = aabb;
    left->max[axis] = pos;
    right->min[axis] = pos;
}

inline float UnitCost(float surfaceRatioL, float surfaceRatioR, float leftTriangles, float righTriangles, bool isEdge)
{
    float cost = K_TRAVERSAL + K_INTERSECTION * (surfaceRatioL * leftTriangles + surfaceRatioR * righTriangles);
    if ((leftTriangles == 0 || righTriangles == 0) && !isEdge)
    {
        cost *= LAMBDA_BIAS;
    }
    return cost;
}

inline float SurfaceAreaHeuristics(const AABB& parentVoxel, float rcpParentSurface, float pos, uint8_t axis, size_t leftTriangles, size_t rightTriangles, float planarTriangles, SplitSide* side)
{
    bool isEdge = (pos == parentVoxel.min[axis] || pos == parentVoxel.max[axis]);
    if (isEdge)
        return std::numeric_limits<float>::infinity();
    AABB left, right;
    SplitBox(parentVoxel, pos, axis, &left, &right);
    float pLeft = CalculateSurfaceArea(left) * rcpParentSurface;
    float pRight = CalculateSurfaceArea(right) * rcpParentSurface;
    float leftCost = UnitCost(pLeft, pRight, leftTriangles + planarTriangles, rightTriangles, isEdge);
    float rightCost = UnitCost(pLeft, pRight, leftTriangles, rightTriangles + planarTriangles, isEdge);
    //also decide where the planar triangles should go
    if (leftCost > rightCost)
    {
        *side = kSplitSideLeft;
        return leftCost;
    }
    else
    {
        *side = kSplitSideRight;
        return rightCost;
    }
}

//find the best splitting plane
static float findPlane(const std::vector<Triangle>& triangles, const AABB& aabb, const std::vector<SAHEvent>* eventList, Axis* bestAxis, SplitSide* bestSide, float* bestCost)
{
    *bestCost = std::numeric_limits<float>::infinity();
    float rcpParentSurface = 1.0f / CalculateSurfaceArea(aabb);
    float bestSplit = 0.0f;
    for (uint8_t k = kAxisX; k < kAxesCount; ++k)
    {
        const std::vector<SAHEvent>& events = eventList[k];
        int eventsLength = (int)events.size();
        int nr = (int)triangles.size(), np = 0, nl = 0;
        for (int i = 0; i < eventsLength;)
        {
            const SAHEvent& event = eventList[k][i];
            int p_minus = 0, p_planar = 0, p_plus = 0;
            float pos = event.planePosition;
            //count how many events ended, lied in the plane and started
            while (i < eventsLength && events[i].planePosition == pos && events[i].type == kEventEnd)
            {
                i++;
                p_minus++;
            }
            while (i < eventsLength && events[i].planePosition == pos && events[i].type == kEventPlanar)
            {
                i++;
                p_planar++;
            }
            while (i < eventsLength && events[i].planePosition == pos && events[i].type == kEventStart)
            {
                i++;
                p_plus++;
            }
            np = p_planar;
            nr -= p_minus + p_planar;
            AABB left;
            AABB right;

            SplitSide side;
            float cost = SurfaceAreaHeuristics(aabb, rcpParentSurface, pos, k, nl, nr, np, &side);
            if (cost < *bestCost)
            {
                *bestCost = cost;
                *bestSide = side;
                *bestAxis = (Axis)k;
                bestSplit = pos;
            }
            nl += p_plus + p_planar;
        }
    }
    return bestSplit;
}

inline void ClassifyLeftRightBoth(const std::vector<SAHEvent>& axisEvents, float splitPos, int planarSide, std::vector<SplitSide>& sides)
{
    for (int i = 0; i < axisEvents.size(); ++i)
    {
        const SAHEvent& event = axisEvents[i];
        //calculate how many events lie on each side of the split plane and save the side of the triangle for later sorting
        if (event.type == kEventEnd && event.planePosition <= splitPos)
        {
            sides[event.tri] = kSplitSideLeft;
        }
        if (event.type == kEventStart && event.planePosition >= splitPos)
        {
            sides[event.tri] = kSplitSideRight;
        }
        if (event.type == kEventPlanar)
        {
            if ((event.planePosition == splitPos && planarSide == 0) || event.planePosition < splitPos)
            {
                sides[event.tri] = kSplitSideLeft;
            }
            else if ((event.planePosition == splitPos && planarSide != 0) || event.planePosition > splitPos)
            {
                sides[event.tri] = kSplitSideRight;
            }
        }
    }
}

inline void SplitTriangles(const std::vector<Triangle>& faces, const std::vector<SplitSide>& sides, std::vector<Triangle>& Tl, std::vector<Triangle>& Tr, std::vector<int>* triangleMap)
{
    Tl.reserve(faces.size());
    Tr.reserve(faces.size());
    for (int i = 0; i < faces.size(); ++i)
    {
        SplitSide side = sides[i];
        //split the triangle list, while assigning the new id's to a list
        if (side == kSplitSideLeft)
        {
            triangleMap[side][i] = (int)Tl.size();
            Tl.push_back(faces[i]);
        }
        if (side == kSplitSideRight)
        {
            triangleMap[side][i] = (int)Tr.size();
            Tr.push_back(faces[i]);
        }
    }
}

inline void SplitEvents(const std::vector<SAHEvent>* events, const std::vector<SplitSide>& sides, std::vector<SAHEvent>* leftEvents, std::vector<SAHEvent>* rightEvents, const std::vector<int>* triangleMap)
{
    for (int k = 0; k < kAxesCount; ++k)
    {
        leftEvents[k].reserve(events[k].size());
        rightEvents[k].reserve(events[k].size());
        const std::vector<SAHEvent>& axisEvents = events[k];
        for (int i = 0; i < axisEvents.size(); ++i)
        {
            SAHEvent event = axisEvents[i];
            SplitSide side = sides[event.tri];
            //if either left or right side, set the index to the corresponding array's space
            if (side < kSplitSideBoth)
            {
                event.tri = triangleMap[side][event.tri];
            }
            if (side == kSplitSideLeft)
            {
                leftEvents[k].push_back(event);
            }
            else if (side == kSplitSideRight)
            {
                rightEvents[k].push_back(event);
            }
        }
    }
}

//create two new events each time an event is cut in half by the split plane
inline void CreateStrandedEvents(const std::vector<Triangle>& faces, const std::vector<SplitSide>& sides, const AABB& leftAABB, const AABB& rightAABB, std::vector<Triangle>& Tl, std::vector<Triangle>& Tr, std::vector<SAHEvent>* leftSplitEvents, std::vector<SAHEvent>* rightSplitEvents)
{
    for (int i = 0; i < faces.size(); ++i)
    {
        if (sides[i] == kSplitSideBoth)
        {
            const Triangle& tri = faces[i];
            int idL = (int)Tl.size();
            int idR = (int)Tr.size();
            Tl.push_back(tri);
            Tr.push_back(tri);
            for (int k = 0; k < kAxesCount; ++k)
            {
                //if triangle is perpendicular to the axis (=lies inside the split plane), create two planar events
                if (fabsf(tri.GetNormal()[k]) == 1.0)
                {
                    SAHEvent ev;
                    ev.planePosition = tri.GetAxisMin((Axis)k);
                    ev.type = kEventPlanar;

                    ev.tri = idL;
                    leftSplitEvents[k].push_back(ev);

                    ev.tri = idR;
                    rightSplitEvents[k].push_back(ev);
                //otherwise, two starts/ends, two for each side
                }
                else
                {
                    SAHEvent ev0, ev1;
                    ev0.type = kEventStart;
                    ev1.type = kEventEnd;

                    ev0.tri = ev1.tri = idL;
                    ev0.planePosition = std::min(std::max(tri.GetAxisMin((Axis)k), leftAABB.min[k]), leftAABB.max[k]);
                    ev1.planePosition = std::max(std::min(tri.GetAxisMax((Axis)k), leftAABB.max[k]), leftAABB.min[k]);
                    leftSplitEvents[k].push_back(ev0);
                    leftSplitEvents[k].push_back(ev1);

                    ev0.tri = ev1.tri = idR;
                    ev0.planePosition = std::min(std::max(tri.GetAxisMin((Axis)k), rightAABB.min[k]), rightAABB.max[k]);
                    ev1.planePosition = std::max(std::min(tri.GetAxisMax((Axis)k), rightAABB.max[k]), rightAABB.min[k]);
                    rightSplitEvents[k].push_back(ev0);
                    rightSplitEvents[k].push_back(ev1);
                }
            }
        }
    }
}

inline void SortAndInsertSplitEvents(std::vector<SAHEvent>* leftEvents, std::vector<SAHEvent>* rightEvents, std::vector<SAHEvent>* leftSplitEvents, std::vector<SAHEvent>* rightSplitEvents)
{
    for (int k = 0; k < kAxesCount; ++k)
    {

        //We sort these two, but they are usually really small and it is mostly sorted already
        if (leftSplitEvents[k].size())
        {
            std::sort(leftSplitEvents[k].begin(), leftSplitEvents[k].end(), EventSortPredicate);
            std::vector<SAHEvent> tmp(leftEvents[k].size() + leftSplitEvents[k].size());
            std::merge(leftEvents[k].begin(), leftEvents[k].end(), leftSplitEvents[k].begin(), leftSplitEvents[k].end(), tmp.begin(), EventSortPredicate);
            leftEvents[k] = tmp;
        }

        if (rightSplitEvents[k].size())
        {
            std::sort(rightSplitEvents[k].begin(), rightSplitEvents[k].end(), EventSortPredicate);
            std::vector<SAHEvent> tmp(rightEvents[k].size() + rightSplitEvents[k].size());
            std::merge(rightEvents[k].begin(), rightEvents[k].end(), rightSplitEvents[k].begin(), rightSplitEvents[k].end(), tmp.begin(), EventSortPredicate);
            rightEvents[k] = tmp;
        }
    }
}

KDNode* KDNode::CreateNode(const std::vector<Triangle>& faces, const AABB& aabb, const std::vector<SAHEvent>* events, int depth)
{
    Axis axis = kAxesCount;
    SplitSide planarSide = kSplitSideBoth;
    float splitCost;
    float splitPos = findPlane(faces, aabb, events, &axis, &planarSide, &splitCost);

    //C < Kt x |T| is the SAH termination criterion
    if (splitCost < K_INTERSECTION * faces.size())
    {
        KDNode* node = new KDNode();
        node->m_AABB = aabb;
        AABB leftAABB = aabb;
        AABB rightAABB = aabb;
        leftAABB.max[axis] = splitPos;
        rightAABB.min[axis] = splitPos;
        std::vector<Triangle> Tl;
        std::vector<Triangle> Tr;

        std::vector<SAHEvent> leftEvents[kAxesCount];
        std::vector<SAHEvent> rightEvents[kAxesCount];
        {
            std::vector<SplitSide> sides(faces.size(), kSplitSideBoth);

            ClassifyLeftRightBoth(events[axis], splitPos, planarSide, sides);
            std::vector<int> triangleMap[2];
            triangleMap[0].resize(faces.size());
            triangleMap[1].resize(faces.size());
            SplitTriangles(faces, sides, Tl, Tr, triangleMap);
            std::vector<SAHEvent> leftSplitEvents[kAxesCount];
            std::vector<SAHEvent> rightSplitEvents[kAxesCount];
            SplitEvents(events, sides, leftEvents, rightEvents, triangleMap);

            CreateStrandedEvents(faces, sides, leftAABB, rightAABB, Tl, Tr, leftSplitEvents, rightSplitEvents);

            SortAndInsertSplitEvents(leftEvents, rightEvents, leftSplitEvents, rightSplitEvents);
        }
        node->m_Left.reset(CreateNode(Tl, leftAABB, leftEvents, depth + 1));
        node->m_Right.reset(CreateNode(Tr, rightAABB, rightEvents, depth + 1));
        if (node->m_Left || node->m_Right)
            return node;
    }
    else if (faces.size())
    {
        //we found a leaf node
        KDNode* node = new KDNode();
        node->m_AABB = aabb;
        node->m_Left = nullptr;
        node->m_Right = nullptr;
        for (int i = 0; i < faces.size(); ++i)
        {
            const Triangle& tri = faces[i];
            node->triangles.push_back(tri);
        }
        return node;
    }
    return nullptr;
}
