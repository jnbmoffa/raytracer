#pragma once
#include "AxisAlignedBox.h"
#include "array.h"
#include "ray.h"
#include <iostream>

class OcTreeObject
{
public:
    virtual ~OcTreeObject() {}
    // Every octree object needs to have a bouding box
    virtual BoxF GetBox() = 0;
};

// OcTree implementation that holds OctObjectType objects inside of it
// OctObjectType must be a subclass of OcTreeObject
template<typename OctObjectType = OcTreeObject>
class OcTree
{
    unsigned level;         // Depth in octree structure
    BoxF Bounds;            // Bounds of this tree
    Array<OctObjectType*> objects;  // objects in this tree
    Array<OcTree*> nodes;   // regions
    unsigned MAX_OBJECTS, MAX_LEVELS;

    // Break this tree up into 8 cubes
    void Split()
    {
        const double halfWidth = Bounds.GetWidth() * 0.5;
        const double halfHeight = Bounds.GetHeight() * 0.5;
        const double halfDepth = Bounds.GetDepth() * 0.5;

        const double l = Bounds.GetLeft();
        const double r = Bounds.GetRight();
        const double t = Bounds.GetTop();
        const double bo = Bounds.GetBottom();
        const double f = Bounds.GetFront();
        const double ba = Bounds.GetBack();

        // top left front = 0
        nodes.Add(new OcTree(level + 1, BoxF(l + halfWidth, l, t, t - halfHeight, f, f - halfDepth), MAX_OBJECTS, MAX_LEVELS));
        // top right front = 1
        nodes.Add(new OcTree(level + 1, BoxF(r, r - halfWidth, t, t - halfHeight, f, f - halfDepth), MAX_OBJECTS, MAX_LEVELS));
        // top left back = 2
        nodes.Add(new OcTree(level + 1, BoxF(l + halfWidth, l, t, t - halfHeight, ba + halfDepth, ba), MAX_OBJECTS, MAX_LEVELS));
        // top right back = 3
        nodes.Add(new OcTree(level + 1, BoxF(r, r - halfWidth, t, t - halfHeight, ba + halfDepth, ba), MAX_OBJECTS, MAX_LEVELS));

        // bottom left front = 4
        nodes.Add(new OcTree(level + 1, BoxF(l + halfWidth, l, bo + halfHeight, bo, f, f - halfDepth), MAX_OBJECTS, MAX_LEVELS));
        // bottom right front = 5
        nodes.Add(new OcTree(level + 1, BoxF(r, r - halfWidth, bo + halfHeight, bo, f, f - halfDepth), MAX_OBJECTS, MAX_LEVELS));
        // bottom left back = 6
        nodes.Add(new OcTree(level + 1, BoxF(l + halfWidth, l, bo + halfHeight, bo, ba + halfDepth, ba), MAX_OBJECTS, MAX_LEVELS));
        // bottom right back = 7
        nodes.Add(new OcTree(level + 1, BoxF(r, r - halfWidth, bo + halfHeight, bo, ba + halfDepth, ba), MAX_OBJECTS, MAX_LEVELS));
    }

    // Return index of the cube the box fits into
    // -1 means it doesn't fit nicely into a child
    int GetIndex(const BoxF& pBox)
    {
        int index = -1;
        double vertMidpoint = Bounds.GetLeft() + (Bounds.GetWidth() * 0.5);
        double horiMidpoint = Bounds.GetBottom() + (Bounds.GetHeight() * 0.5);
        double depthMidpoint = Bounds.GetBack() + (Bounds.GetDepth() * 0.5);

        // Object can completely fit within the top regions
        bool topHalf = pBox.GetBottom() > horiMidpoint;
        // Object can completely fit within the bottom regions
        bool botHalf = pBox.GetTop() < horiMidpoint;

        // Object can completely fit in front regions
        bool frontHalf = pBox.GetBack() > depthMidpoint;
        // Object can completely fit in back regions
        bool backHalf = pBox.GetFront() < depthMidpoint;

        if (pBox.GetRight() < vertMidpoint) // left half
        {
            if (topHalf)    // top left
            {
                if (frontHalf)
                {
                    index = 0;
                }
                else if (backHalf)
                {
                    index = 2;
                }
            }
            else if (botHalf)   // bottom left
            {
                if (frontHalf)
                {
                    index = 4;
                }
                else if (backHalf)
                {
                    index = 6;
                }
            }
        }
        else if (pBox.GetLeft() > vertMidpoint) // right half
        {
            if (topHalf)    // top right
            {
                if (frontHalf)
                {
                    index = 1;
                }
                else if (backHalf)
                {
                    index = 3;
                }
            }
            else if (botHalf)   // bottom right
            {
                if (frontHalf)
                {
                    index = 5;
                }
                else if (backHalf)
                {
                    index = 7;
                }
            }
        }

        return index;
    }

    OcTree(int pLevel, const BoxF& Bounds, int maxObj, int maxLvl) :
        level(pLevel),
        Bounds(Bounds),
        MAX_OBJECTS(maxObj),
        MAX_LEVELS(maxLvl)
    {}

public:
    OcTree() :
        level(0),
        MAX_OBJECTS(1),
        MAX_LEVELS(4)
    {}

    OcTree(const BoxF& Bounds, int maxObj = 1, int maxLvl = 4) :
        level(0),
        Bounds(Bounds),
        MAX_OBJECTS(maxObj),
        MAX_LEVELS(maxLvl)
    {}

    ~OcTree()
    {
        Clear();
    }

    OcTree& operator=(const OcTree& RHS)
    {
        if (&RHS == this)
        {
            return *this;
        }

        Bounds = BoxF(RHS.Bounds.GetRight(), RHS.Bounds.GetLeft(), RHS.Bounds.GetTop(), RHS.Bounds.GetBottom(),
                      RHS.Bounds.GetFront(), RHS.Bounds.GetBack());

        // Reset this tree
        Clear();

        // Insert the other tree's objects into this one
        for (unsigned int i = 0; i < RHS.objects.Num(); i++)
        {
            this->Insert(RHS.objects[i]);
        }

        level = RHS.level;

        return *this;
    }

    // Remove all objects and nodes from the entire tree (including sub-trees)
    void Clear()
    {
        objects.Clear();

        while (nodes.Num() > 0)
        {
            nodes[0]->Clear();
            delete nodes[0];
            nodes.RemoveAtSwap(0);
        }
    }

    // Add an object to the tree
    void Insert(OctObjectType* Object)
    {
        BoxF pBox = Object->GetBox();

        // Have children? Try to put it in one of them
        if (nodes.Num() > 0)
        {
            int index = GetIndex(pBox);

            if (index != -1)
            {
                nodes[index]->Insert(Object);
                return;
            }
        }

        // Put it in me
        objects.Add(Object);

        // Full? split and try to distribute objects into children
        if (objects.Num() > MAX_OBJECTS && level < MAX_LEVELS)
        {
            if (nodes.Num() == 0)
            {
                Split();
            }

            unsigned int i = 0;
            while (i < objects.Num())
            {
                int index = GetIndex(objects[i]->GetBox());

                if (index != -1)
                {
                    nodes[index]->Insert(objects[i]);
                    objects.RemoveAtSwap(i);
                }
                else
                {
                    i++;
                }
            }
        }
    }

    // Get the objects nearby the provided rect in the tree
    void Retrieve(Array<OctObjectType*>& returnObjects, BoxF ObjectRect)
    {
        int index = GetIndex(ObjectRect);

        // Have subnodes
        if (nodes.Num() > 0)
        {
            // Object box fits into a subnode then retrieve objects from that node
            if (index != -1)
            {
                nodes[index]->Retrieve(returnObjects, ObjectRect);
                return;
            }

            // doesn't fit into a subnode so check every subnode
            else
            {
                for (unsigned int i = 0; i < nodes.Num(); i++)
                {
                    nodes[i]->Retrieve(returnObjects, ObjectRect);
                }
            }
        }

        for (unsigned int i = 0; i < objects.Num(); i++)
        {
            returnObjects.Add(objects[i]);
        }

        return;
    }

    // Get the objects that might be hit by the ray
    bool Trace(Array<OctObjectType*>& returnObjects, const Ray& r) const
    {
        if ((nodes.Num() > 0 || objects.Num() > 0) && CheckIntersection(r, Bounds))
        {
            // check every subnode
            for (OcTree* T : nodes)
            {
                T->Trace(returnObjects, r);
            }

            // Add our objects
            for (OctObjectType* O : objects)
            {
                returnObjects.Add(O);
            }

            return true;
        }

        return false;
    }

    template<typename T>
    friend std::ostream& operator <<(std::ostream& os, const OcTree<T>& B);
};

template<typename T>
std::ostream& operator <<(std::ostream& os, const OcTree<T>& B)
{
    if (B.nodes.Num() == 0)
    {
        os << "Num objects: " << B.objects.Num();
        for (int i = 0; i < B.objects.Num(); i++)
        {
            os << std::endl;
            for (int j = 0; j < B.level; j++)
            {
                os << "  ";
            }
            os << B.objects[i]->GetBox();
        }
    }
    else
    {
        for (int i = 0; i < B.nodes.Num(); i++)
        {
            os << std::endl;
            for (int i = 0; i < B.level; i++)
            {
                os << "  ";
            }
            os << "Tree (" << B.level << "," << i << "): " << *(B.nodes[i]);
        }
    }
    return os;
}