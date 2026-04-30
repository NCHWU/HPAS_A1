#pragma once


#include "TSNEGradientCompute.h"
#include <framework/NDArray.h>
#include "../../ft/ft.h"

//#define DO_REFERENCE_TESTING

class FItSNE : public TSNEGradientCompute
{
public:
    FItSNE();
    ~FItSNE() = default;

    virtual void computeNegativeGradient(NDArray<vec>& points, NDArray<vec>& gradient, DebugRenderData& debugRenderData) override;

    virtual vec lagrangeInterpolate(vec pointInInterval, int interpolationPointIndex,
        NDArray<prec_float>& lagrangeDenominator, NDArray<prec_float>& interpolationPointsInInterval, prec_float& min_coord, prec_float& max_coord);

    virtual void precalculate_q_denom(NDArray<vec>& points, DebugRenderData& debugRenderData) override;

protected:
    /*
    The interpolation grid is a 2D square grid of 'interpolation points' that covers all data points.
	The grid is divided into 'intervals' along each axis, and each interval contains interpolation points.
    */
	int numInterpolationPoints; // number of interpolation points in a single interval along one axis (n_interpolation_points)
    size_t numIntervals; // number of intervals along one axis (n_boxes_per_dim)
	/* For example, if we have 4 intervals along one axis and 4 interpolation points in each interval, the grid would look like this:
	* then numIntervals = 4, numInterpolationPoints = 4
    * Note that this example does not accurately depict the locations of the interpolation points,
    * since they will all be evenly spaced in the actual grid.
    -----------------------------------------
    | . . . . | . . . . | . . . . | . . . . |
    | . . . . | . . . . | . . . . | . . . . |
    | . . . . | . . . . | . . . . | . . . . |
    | . . . . | . . . . | . . . . | . . . . |
    |---------|---------|---------|---------|
    | . . . . | . . . . | . . . . | . . . . |
    | . . . . | . . . . | . . . . | . . . . |
    | . . . . | . . . . | . . . . | . . . . |
    | . . . . | . . . . | . . . . | . . . . |
    |---------|---------|---------|---------|
    | . . . . | . . . . | . . . . | . . . . |
    | . . . . | . . . . | . . . . | . . . . |
    | . . . . | . . . . | . . . . | . . . . |
    | . . . . | . . . . | . . . . | . . . . |
    |---------|---------|---------|---------|
    | . . . . | . . . . | . . . . | . . . . |
    | . . . . | . . . . | . . . . | . . . . |
    | . . . . | . . . . | . . . . | . . . . |
    | . . . . | . . . . | . . . . | . . . . |
    -----------------------------------------
    */
    const int n_terms = 2 + 2;

    // could be private from here, but access to the functions is required for testing
public:
    virtual NDArray<prec_float> precalculateLagrangeDenominator(NDArray<prec_float>& interpolationPointsInInterval);

    virtual NDArray<vec> calculateIntervalsLowerBounds(prec_float min_coord, prec_float max_coord);
    virtual NDArray<vec> calculatePointsRelativeToGrid(NDArray<vec>& points, prec_float min_coord, prec_float max_coord);
	virtual NDArray<glm::ivec2> calculateIntervalIndices(NDArray<vec>& pointsRelativeToGrid);
    virtual NDArray<vec> calculatePointsRelativeToInterval(NDArray<vec>& pointsRelativeToGrid, NDArray<glm::ivec2>& intervalIndices);

    virtual NDArray<vec> calculateInterpolationPoints(NDArray<vec>& intervalsLowerBounds, prec_float gridWidth);

    virtual NDArray<double> calculateCharges(NDArray<vec>& points);

    virtual NDArray<prec_float> calculateWCoefficients(
        NDArray<vec>& points,
        NDArray<glm::ivec2>& intervalIndices,
        NDArray<vec>& lagrangeInterpolatedValues,
        NDArray<prec_float>& charges);

    virtual NDArray<prec_float> calculateKernel(NDArray<vec>& interpolationPoints);

    virtual NDArray<prec_float> calculatePotentialsQij(
        NDArray<vec>& points,
        NDArray<prec_float>& kernel,
        NDArray<prec_float>& w,
        NDArray<glm::ivec2>& intervalIndices,
        NDArray<vec>& lagrangeInterpolatedValues);

	void visualizeWCoefficients(NDArray<prec_float>& wCoefficients, NDArray<vec>& interpolationPoints, DebugRenderData& debugRenderData);

    virtual double calculateSumQ(NDArray<vec>& points, NDArray<prec_float>& potentialsQij);
};

inline prec_float cauchyKernel(vec y, vec z) {
    //return 1 / (1 + (y.x - y.y) * (y.x - y.y) + (z.x - z.y) * (z.x - z.y));
    return 1 / (1 + (y.x - z.x) * (y.x - z.x) + (y.y - z.y) * (y.y - z.y));
}

inline prec_float cauchyKernelSquared(vec y, vec z) {
	prec_float dx = y.x - z.x;
    prec_float dy = y.y - z.y;
    prec_float denom = (1 + dx * dx + dy * dy);
    return 1 / (denom * denom);
}