#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <vector>
#include <omp.h>
#include <iostream>

#include "framework/NDArray.h"
#include "framework/DebugRenderData.h"
#include "framework/PMatrixCalculator.h"

#define TSNE_PRECISION 2

typedef DebugRenderData2D DebugRenderData;

#if TSNE_PRECISION == 1
typedef float prec_float;
typedef glm::vec2 vec;
#elif TSNE_PRECISION == 2
typedef double prec_float;
typedef glm::dvec2 vec;
#endif

#define START_TIMER() double time = getTime();
#define RESET_TIMER() time = getTime();
#define LOG_TIME(msg) \
    std::cout << msg << ": " << (getTime() - time) << " s" << std::endl; \
    time = getTime();

class TSNEGradientCompute
{
public:
	TSNEGradientCompute() {}

	virtual ~TSNEGradientCompute() {}

	// Compute the positive gradient of the t-SNE algorithm
	virtual void computePositiveGradient(NDArray<vec>& points, NDArray<vec>& gradient, PMatrix& p_matrix, prec_float exaggerationFactor, DebugRenderData& debugRenderData);

	// Compute the negative gradient of the t-SNE algorithm
	virtual void computeNegativeGradient(NDArray<vec>& points, NDArray<vec>& gradient, DebugRenderData& debugRenderData);

	virtual void precalculate_q_denom(NDArray<vec>& points, DebugRenderData& debugRenderData);

	// Setup this computer for the next step
	virtual void setup(NDArray<vec>& points, DebugRenderData& debugRenderData);

	// Reset this computer for the next step
	virtual void reset();

protected:

	// Get the current time in seconds from the start of the application
	double getTime();

	// Clamp a value between a minimum and maximum
	prec_float clamp(prec_float x, prec_float min, prec_float max);
	static prec_float q_numerator(vec& a, vec& b);
	static prec_float sqdistance(vec& a, vec& b);
	static prec_float q_denom(vec& a, vec& b);

	// Find the negative gradient for two points
	static vec negative_gradient(vec& a, vec& b);

	prec_float q_denom_precalculated{ 0.0f };
};
