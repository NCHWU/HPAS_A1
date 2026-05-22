#pragma once

#include "TSNEGradientCompute.h"
#include "QuadTree.h"

class QuadtreeTSNE : public TSNEGradientCompute
{
public:
	QuadtreeTSNE();
	~QuadtreeTSNE();

	// Compute the negative gradient of the t-SNE algorithm
	virtual void computeNegativeGradient(NDArray<vec>& points, NDArray<vec>& gradient, DebugRenderData& debugRenderData) override;

	virtual void precalculate_q_denom(NDArray<vec>& points, DebugRenderData& debugRenderData) override;

	// Setup this computer for the next step
	virtual void setup(NDArray<vec>& points, DebugRenderData& debugRenderData) override;

	// Reset this computer for the next step
	virtual void reset() override;

	int bucketSize{ 100 };
	float theta{ 0.6f };

private:
	Node root;
};