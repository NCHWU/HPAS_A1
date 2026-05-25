#include "QuadtreeGradientCompute.h"

QuadtreeTSNE::QuadtreeTSNE()
{}

QuadtreeTSNE::~QuadtreeTSNE()
{}

void QuadtreeTSNE::precalculate_q_denom(NDArray<vec>& points, DebugRenderData& debugRenderData)
{
	size_t numPoints = points.size();

	bool debugRenderDataEnabled = debugRenderData.enabled;
	// This needs to be set to false if you are using omp parallel, since debugRenderData is not thread safe
	debugRenderData.enabled = false;

	q_denom_precalculated = 0.0f;

	// Implement the q_denom_precalculated computation using the quadtree her

    for(size_t i = 0; i < numPoints; i++){
        q_denom_precalculated += root.sampleQDenom(points, points(i), static_cast<int>(i), theta);
    }

    debugRenderData.enabled = debugRenderDataEnabled;

}

void QuadtreeTSNE::setup(NDArray<vec>& points, DebugRenderData& debugRenderData)
{
	double startTime = getTime();
	root.build(points, bucketSize);
	double endTime = getTime();
	std::cout << "Building tree took " << endTime - startTime << "s" << std::endl;
	precalculate_q_denom(points, debugRenderData);
}

void QuadtreeTSNE::reset()
{
	root.clear();
}

void QuadtreeTSNE::computeNegativeGradient(NDArray<vec>& points, NDArray<vec>& gradient, DebugRenderData& debugRenderData)
{
	size_t numPoints = points.size();

	prec_float Z = q_denom_precalculated;

	bool debugRenderDataEnabled = debugRenderData.enabled;
	// This needs to be set to false if you are using omp parallel, since debugRenderData is not thread safe
	debugRenderData.enabled = false;

	// Implement the negative gradient computation using the quadtree here
	for(size_t i = 0; i < numPoints; i++){
        vec negGrad = root.sampleNegGradient(points, points(i), static_cast<int>(i), theta);
        negGrad /= Z;
        negGrad *= 4.0;
        gradient(i) -= negGrad;
    }

	debugRenderData.enabled = debugRenderDataEnabled;
}
