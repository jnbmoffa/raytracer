#pragma once
#include "array.h"

template<typename T>
class Box
{
	T left;
	T right;
	T top;
	T bottom;
	T front;
	T back;

public:
	Rect() :
		left(0.f), right(0.f), top(0.f), bottom(0.f), front(0.f), back(0.f) {}

	Rect(T left, T right, T top, T bottom, T front, T back) :
		left(left), right(right), top(top), bottom(bottom), front(front), back(back) {}

	inline T GetLeft() const
	{
		return left;
	}

	inline T GetRight() const
	{
		return right;
	}

	inline T GetTop() const
	{
		return top;
	}

	inline T GetBottom() const
	{
		return bottom;
	}

	inline T GetFront() const
	{
		return front;
	}

	inline T GetBack() const
	{
		return back;
	}

	inline T GetWidth() const
	{
		return abs(right - left);
	}

	inline T GetHeight() const
	{
		return abs(top - bottom);
	}

	inline T GetDepth() const
	{
		return abs(front - back);
	}
};

typedef Box<double> BoxF;

class QuadTreeObject
{
public:
	// Every quad tree object needs to have a bouding box
	virtual BoxF GetBox() = 0;
};

// Quad tree implementation that holds QuadObjectType objects inside of it
// QuadObjectType must be a subclass of QuadTreeObject
template<typename QuadObjectType = QuadTreeObject>
class QuadTree
{
	static const int MAX_OBJECTS = 1;
	static const int MAX_LEVELS = 4;

	int level;				// Depth in quad tree structure
	BoxF Bounds;			// Bounds of this tree
	Array<QuadObjectType*> objects;	// objects in this tree
	Array<QuadTree*> nodes;	// quadrants
	
	// Break this tree up into four quadrants
	void Split()
	{
		float halfWidth = Bounds.GetWidth() / 2;
		float halfHeight = Bounds.GetHeight() / 2;
		float x = Bounds.GetLeft();
		float r = Bounds.GetRight();
		float y = Bounds.GetTop();
		float b = Bounds.GetBottom();

		// top left = 0
		nodes.Add(new QuadTree(level + 1, BoxF(x, y, x + halfWidth, y + halfHeight)));
		// top right = 1
		nodes.Add(new QuadTree(level + 1, BoxF(x + halfWidth, y, r, y + halfHeight)));
		// bottom left  = 2
		nodes.Add(new QuadTree(level + 1, BoxF(x, y + halfHeight, x + halfWidth, b)));
		// bottom right  = 3
		nodes.Add(new QuadTree(level + 1, BoxF(x + halfWidth, y + halfHeight, r, b)));
	}

	// Return index of quadrant the rect fits into
	int GetIndex(const BoxF& pBox)
	{
		int index = -1;
		double vertMidpoint = Bounds.GetLeft() + Bounds.GetWidth() / 2;
		double horiMidpoint = Bounds.GetTop() + Bounds.GetHeight() / 2;

		// Object can completely fit within the top quadrants
		bool topHalf = (pBox.GetBottom() < horiMidpoint);
		// Object can completely fit within the bottom quadrants
		bool botHalf = (pBox.GetTop() > horiMidpoint);

		// Fits in left half
		if (pBox.GetRight() < vertMidpoint)
		{
			if (topHalf)	// top left
				index = 0;
			else if (botHalf)	// bottom left
				index = 2;
		}
		// Fits in right half
		else if (pBox.GetLeft() > vertMidpoint)
		{
			if (topHalf)	// top right
				index = 1;
			else if (botHalf)	// bottom right
				index = 3;
		}

		return index;
	}

public:
	QuadTree() {}
	QuadTree(int pLevel, const RectF& Bounds) : level(pLevel), Bounds(Bounds) {}
	~QuadTree()
	{
		this->Clear();
	}

	QuadTree& operator=(const QuadTree& RHS)
	{
		if (&RHS == this) return *this;

		Bounds = BoxF(RHS.Bounds.GetLeft(), RHS.Bounds.GetTop(), RHS.Bounds.GetRight(), RHS.Bounds.GetBottom());

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
	void Insert(QuadObjectType* Object)
	{
		if (!Object->CanBeAddedToTree())
		{
			return;
		}

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
					objects.RemoveAt(i);
				}
				else
				{
					i++;
				}
			}
		}
	}

	// Get the objects nearby the provided rect in the tree
	void Retrieve(Array<QuadObjectType*>& returnObjects, BoxF ObjectRect)
	{
		int index = GetIndex(ObjectRect);

		// Have subnodes
		if (nodes.Num() > 0)
		{
			// ObjectRect fits into a subnode then retrieve objects from that node
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

	// // Return an array of bounding rectangle for all levels of the tree
	// void GetBounds(Array<BoxF>& BoundsArray)
	// {
	// 	BoundsArray.Add(Bounds);

	// 	for (unsigned int i = 0; i < nodes.Num(); i++)
	// 	{
	// 		nodes[i]->GetBounds(BoundsArray);
	// 	}
	// }
};