#pragma once
#include "array.h"
#include "kdobject.h"
#include <algorithm>
#include <vector>
#include <limits>

// Comparison function for finding the median for KDTree building
template<typename T>
class AxisCompare
{
	unsigned int Index;

public:
	AxisCompare(unsigned int Index) : Index(Index) {}
	bool operator()(const T* a, const T* b) const
	{
		// Compare only the index associted with the level of the tree
		return (*a)[Index] < (*b)[Index];
	}
};

// General KDTree
// Only implements the required functionality for photon mapping.
// Most of the concepts were taken from these papers:
// http://www.cs.cmu.edu/afs/cs/academic/class/15462-f14/www/lec_slides/a1-jensen.pdf
// Multidimensional Binary Search Trees Used for Associative Searching, Jon Louis Bently, Stanford, 1975
template<typename KDType = KDObject>
class KDTree
{
	unsigned int K;

	struct Node
	{
		Node() : P(nullptr), Left(nullptr), Right(nullptr) {}
		Node(KDType* P, Node *Left, Node *Right, unsigned int K) : K(K), P(P), Left(Left), Right(Right) {}

		inline unsigned int CountNodes()
		{
			return 1 + (Left ? Left->CountNodes() : 0) + (Right ? Right->CountNodes() : 0);
		}

		// Find all KDType elements within SearchDistSq square units from the CheckLoc
		// MaxDist2 will be the square dist to the furthest away KDType element returned
		void LocateNearby(Array<KDType*>& OutArray, const KDType& CheckLoc, const double& SearchDistSq, double& MaxDist2, int depth = 0)
		{
			int axis = depth % K;
			if (Left || Right)
			{
				double delta = CheckLoc[axis] - (*P)[axis];
				if (delta < 0)
				{
					// Left plane, check left subtree first
					if (Left) Left->LocateNearby(OutArray, CheckLoc, SearchDistSq, MaxDist2, depth+1);
					if (delta*delta < SearchDistSq)
						if (Right) Right->LocateNearby(OutArray, CheckLoc, SearchDistSq, MaxDist2, depth+1);
				}
				else
				{
					// Right plane, check right subtree first
					if (Right) Right->LocateNearby(OutArray, CheckLoc, SearchDistSq, MaxDist2, depth+1);
					if (delta*delta < SearchDistSq)
						if (Left) Left->LocateNearby(OutArray, CheckLoc, SearchDistSq, MaxDist2, depth+1);
				}
			}
			
			double D2 = 0.f, val;
			for (unsigned int i=0;i<K;i++)
			{
				val = (*P)[i] - CheckLoc[i];
				D2 += val*val;
			}
			if (D2 < SearchDistSq)
			{
				OutArray.Add(P);
				if (D2 > MaxDist2) MaxDist2 = D2;
			}
		}

		unsigned int K;
		KDType* P;
		Node *Left, *Right;
	} *Root;

	// Inserion into KDTree from array adapted from Wikipedia
	// Split the array on the median, storing it in the current node and recursing on the other halves of the array
	Node* Internal_MakeTree(std::vector<KDType*>& List, int depth)
	{
		if (List.size() == 0) return nullptr;
		if (List.size() == 1) return new Node(List[0], nullptr, nullptr, K);
		int axis = depth % K;

		// Select Median
		unsigned int median = List.size()/2.f;
		std::nth_element(List.begin(), List.begin() + List.size()/2, List.end(), AxisCompare<KDType>(axis));

		Node* node = new Node;
		node->P = List[median];
		node->K = K;
		std::vector<KDType*> Left(List.begin(), List.begin() + List.size()/2.f);
		std::vector<KDType*> Right(List.begin() + List.size()/2.f + 1, List.end());
		node->Left = Internal_MakeTree(Left, depth+1);
		node->Right = Internal_MakeTree(Right, depth+1);
		return node;
	}

	// Free tree memory
	void CleanupNode(Node* node)
	{
		if(node)
		{
			if(node->Left) CleanupNode(node->Left);
			if(node->Right) CleanupNode(node->Right);
			delete node;
		}
	}

public:
	// Iintialize to K dimensions
	KDTree(unsigned int K) : K(K), Root(nullptr) {}

	~KDTree()
	{
		CleanupNode(Root);
	}

	// Inserion into KDTree from array adapted from Wikipedia
	// Split the array on the median, storing it in the current node and recursing on the other halves of the array
	void MakeTree(std::vector<KDType*>& List)
	{
		if (List.size() == 0) return;
		if (List.size() == 1) { Root = new Node(List[0], nullptr, nullptr, K); return; }

		// Select Median
		unsigned int median = List.size()/2.f;
		std::nth_element(List.begin(), List.begin() + List.size()/2.f - 1, List.end(), AxisCompare<KDType>(0));

		Root = new Node(List[median], nullptr, nullptr, K);
		std::vector<KDType*> Left(List.begin(), List.begin() + List.size()/2.f);
		std::vector<KDType*> Right(List.begin() + List.size()/2.f + 1, List.end());
		Root->Left = Internal_MakeTree(Left, 1);
		Root->Right = Internal_MakeTree(Right, 1);
	}

	unsigned int CountNodes()
	{
		if (Root) return Root->CountNodes();
		return 0;
	}

	// Find all KDType elements within SearchDistSq square units from the CheckLoc
	// MaxDist2 will be the square dist to the furthest away KDType element returned
	void LocateNearby(Array<KDType*>& OutArray, const KDType& CheckLoc, const double& SearchDistSq, double& MaxDist2)
	{
		if(Root) Root->LocateNearby(OutArray, CheckLoc, SearchDistSq, MaxDist2);
	}
};