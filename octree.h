#pragma once
#include "box.h"
#include "array.h"
#include <iostream>

class OcTreeObject
{
public:
	// Every quad tree object needs to have a bouding box
	virtual BoxF GetBox() = 0;
};

// Quad tree implementation that holds OctObjectType objects inside of it
// OctObjectType must be a subclass of OcTreeObject
template<typename OctObjectType = OcTreeObject>
class OcTree
{
	static const int MAX_OBJECTS = 1;
	static const int MAX_LEVELS = 4;

	int level;				// Depth in quad tree structure
	BoxF Bounds;			// Bounds of this tree
	Array<OctObjectType*> objects;	// objects in this tree
	Array<OcTree*> nodes;	// regions
	
	// Break this tree up into 8 cubes
	void Split()
	{
		float halfWidth = Bounds.GetWidth() / 2.f;
		float halfHeight = Bounds.GetHeight() / 2.f;
		float halfDepth = Bounds.GetDepth() / 2.f;
		float l = Bounds.GetLeft();
		float r = Bounds.GetRight();
		float t = Bounds.GetTop();
		float bo = Bounds.GetBottom();
		float f = Bounds.GetFront();
		float ba = Bounds.GetBack();

		// std::cout << level << std::endl;

		// top left front = 0
		nodes.Add(new OcTree(level + 1, BoxF(l, l + halfWidth, t, t - halfHeight, f, f - halfDepth)));
		// top right front = 1
		nodes.Add(new OcTree(level + 1, BoxF(r - halfWidth, r, t, t - halfHeight, f, f - halfDepth)));
		// top left back = 2
		nodes.Add(new OcTree(level + 1, BoxF(l, l + halfWidth, t, t - halfHeight, ba + halfDepth, ba)));
		// top right back = 3
		nodes.Add(new OcTree(level + 1, BoxF(r - halfWidth, r, t, t - halfHeight, ba + halfDepth, ba)));

		// bottom left front = 4
		nodes.Add(new OcTree(level + 1, BoxF(l, l + halfWidth, bo + halfHeight, bo, f, f - halfDepth)));
		// bottom right front = 5
		nodes.Add(new OcTree(level + 1, BoxF(r - halfWidth, r, bo + halfHeight, bo, f, f - halfDepth)));
		// bottom left back = 6
		nodes.Add(new OcTree(level + 1, BoxF(l, l + halfWidth, bo + halfHeight, bo, ba + halfDepth, ba)));
		// bottom right back = 7
		nodes.Add(new OcTree(level + 1, BoxF(r - halfWidth, r, bo + halfHeight, bo, ba + halfDepth, ba)));
	}

	// Return index of region the box fits into
	int GetIndex(const BoxF& pBox)
	{
		int index = -1;
		double vertMidpoint = Bounds.GetLeft() + (Bounds.GetWidth() / 2.f);
		double horiMidpoint = Bounds.GetBottom() + (Bounds.GetHeight() / 2.f);
		double depthMidpoint = Bounds.GetBack() + (Bounds.GetDepth() / 2.f);

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
			if (topHalf)	// top left
			{
				if (frontHalf)
					index = 0;
				else if (backHalf)
					index = 2;
			}
			else if (botHalf)	// bottom left
			{
				if (frontHalf)
					index = 4;
				else if (backHalf)
					index = 6;
			}
		}
		else if (pBox.GetLeft() > vertMidpoint) // right half
		{
			if (topHalf)	// top right
			{
				if (frontHalf)
					index = 1;
				else if (backHalf)
					index = 3;
			}
			else if (botHalf)	// bottom right
			{
				if (frontHalf)
					index = 5;
				else if (backHalf)
					index = 7;
			}
		}

		return index;
	}

public:
	OcTree() {}
	OcTree(const BoxF& Bounds) : level(0), Bounds(Bounds) {  }
	OcTree(int pLevel, const BoxF& Bounds) : level(pLevel), Bounds(Bounds) {  }
	~OcTree()
	{
		this->Clear();
	}

	OcTree& operator=(const OcTree& RHS)
	{
		if (&RHS == this) return *this;

		Bounds = BoxF(RHS.Bounds.GetLeft(), RHS.Bounds.GetRight(), RHS.Bounds.GetTop(), RHS.Bounds.GetBottom(),
					  RHS.Bounds.GetFont(), RHS.Bounds.GetBack());

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
			nodes.RemoveAt(0);
		}
	}

	// Add an object to the tree
	void Insert(OctObjectType* Object)
	{
		BoxF pBox = Object->GetBox();

		if (nodes.Num() > 0){
			int index = GetIndex(pBox);

			if (index != -1){
				nodes[index]->Insert(Object);
				return;
			}
		}

		objects.Add(Object);

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
			else {
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
	bool Trace(Array<OctObjectType*>& returnObjects, const Ray& r)
	{
		if ( (nodes.Num() > 0 || objects.Num() > 0) && Bounds.Intersects(r))
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
};