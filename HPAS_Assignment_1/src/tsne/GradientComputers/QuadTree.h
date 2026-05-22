#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Node
{
	Node* children[4];

	Node()
	{

	}

	/*
	* Build the quadtree from the given points.
	* Assume that this tree was cleared before calling this function and thus all children are nullptr
	*/
	void build(NDArray<vec> points, int bucketSize)
	{

	}

	void clear()
	{
		for (int i = 0; i < 4; i++)
		{
			if (children[i] != nullptr)
			{
				children[i]->clear();
				delete children[i];
				children[i] = nullptr;
			}
		}
	}
};
