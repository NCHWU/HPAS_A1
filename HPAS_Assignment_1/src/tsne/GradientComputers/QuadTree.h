#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Node
{
	Node* children[4]{};


	vec boxMin, boxMax;
	int count = 0;
	vec centerOfMass = vec(0);
	std::vector<int> pointIndices;

	

	/*
	* Build the quadtree from the given points.
	* Assume that this tree was cleared before calling this function and thus all children are nullptr
	*/
	void build(NDArray<vec> points, int bucketSize)
	{
		// Compute global bounding box
		size_t N = points.size();
		if (N == 0) {return;}
		vec boxMin = points(0);
		vec boxMax = points(0);

		for (size_t i = 1; i < N; i++) {
			boxMin = glm::min(boxMin, points(i));
			boxMax = glm::max(boxMax, points(i));
		}

		// make bounding box squared
		vec size = boxMax - boxMin;
		prec_float sideLength = std::max(size.x, size.y);
		vec center = (boxMax + boxMin) * 0.5;
		vec half = vec(sideLength * 0.5) * (1+ 1e-9); // points strictly inside the boundary
		boxMin = center - half;
		boxMax = center + half;

		std::vector<int> indices(N);
		for (size_t i = 0; i < N; i++) {
			indices[i] = static_cast<int>(i);
		}

		this->boxMin = boxMin;
		this->boxMax = boxMax;

		buildRecursive(points, indices, boxMin, boxMax, bucketSize);

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

	void makeLeaf(NDArray<vec>& points, const std::vector<int>& indices)
	{
		this->pointIndices = indices;
		vec sum(0);
		for(int i : indices){
			sum += points(i);
		}
		this->centerOfMass = sum / static_cast<prec_float>(indices.size());
	}

	void buildRecursive(NDArray<vec>& points, const std::vector<int>& indices, vec boxMin, vec boxMax, int bucketSize)
	{
		// record nodes' position and size
		this->boxMin = boxMin;
		this->boxMax = boxMax;
		this->count = static_cast<int>(indices.size());

		// when box is smal, force it to become a leaf
		prec_float boxWidth = boxMax.x - boxMin.x;
		if(boxWidth < 1e-12){
			makeLeaf(points, indices);
			return;
		}

		if(this->count <= bucketSize){
			makeLeaf(points, indices);
			return;
		}

		vec center = (boxMax + boxMin) * 0.5;

		// bucket each point into one of the 4 quadrants
		std::vector<int> childIndices[4];
		for(int i : indices){
			vec p = points(i);
			int k = (p.x >=center.x ? 1 : 0) + (p.y >= center.y ? 2 : 0);
			childIndices[k].push_back(i);
		}

		//recurse into non-empty children
		for(int k = 0; k < 4; k++){
			if(childIndices[k].empty()){
				continue;
			}
			bool top = (k & 2) != 0; 
			bool right = (k & 1) != 0;

			vec childMin (right ? center.x : boxMin.x, top ? center.y : boxMin.y);
			vec childMax (right ? boxMax.x : center.x, top ? boxMax.y : center.y);

			children[k] = new Node();
			children[k]->buildRecursive(points, childIndices[k], childMin, childMax, bucketSize);
		}

		// aggregate childrens' centers of mass
		vec weighted(0);
		for(int k = 0; k < 4; k++){
			if(children[k] == nullptr){
				continue;
			}
			weighted += static_cast<prec_float>(children[k]->count) * children[k]->centerOfMass;
		}
		this->centerOfMass = weighted / static_cast<prec_float>(this->count);
	}

	prec_float sampleQDenom(NDArray<vec>& points, vec queryPoint, int queryIndex, prec_float theta){
		if(!pointIndices.empty()){
			prec_float sum = 0.0;
			for(int k : pointIndices){
				if(k == queryIndex){
					continue;
				}
				vec diff = queryPoint - points(k);
				prec_float d2 = glm::dot(diff, diff); // squared distance
				sum += 1.0 / (1.0 + d2);
			}
			return sum;
		}

		vec diff = queryPoint - centerOfMass;
		prec_float d2 = glm::dot(diff, diff);
		prec_float w = boxMax.x - boxMin.x;

		if(d2 > 0.0 && (w * w) < theta * theta * d2){
			prec_float q = 1.0 / (1.0 + d2);
			return static_cast<prec_float>(count) * q;
		}

		prec_float sum(0);
		for(int k = 0; k < 4; k++){
			if(children[k] == nullptr){
				continue;
			}
			sum += children[k]->sampleQDenom(points, queryPoint, queryIndex, theta);
		}
		return sum;
	}

	vec sampleNegGradient(NDArray<vec>& points, vec queryPoint, int queryIndex, prec_float theta){
		if(!pointIndices.empty()){
			vec sum(0);
			for (int k : pointIndices){
				if (k == queryIndex){
					continue;
				}
				vec diff = queryPoint - points(k);
				prec_float d2 = glm::dot(diff, diff);
				prec_float q  = 1.0 / (1.0 + d2);
				sum += q * q * (points(k) - queryPoint);  
    		}
        	return sum;
		}
			
		vec diff = queryPoint - centerOfMass;
		prec_float d2 = glm::dot(diff, diff);
		prec_float w  = boxMax.x - boxMin.x;

		if(d2 > 0.0 && (w * w) < theta * theta * d2){
			prec_float q = 1.0 / (1.0 + d2);
			return static_cast<prec_float>(count) * q * q * (centerOfMass - queryPoint);
    	}

		vec sum(0);
		for(int k = 0; k < 4; k++){
			if (children[k] == nullptr){
				continue;
			}
			sum += children[k]->sampleNegGradient(points, queryPoint, queryIndex, theta);
		}
		return sum;
	}

};
