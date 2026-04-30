#include "TSNEGradientCompute.h"

/**
 ====== TODO: IMPLEMENT ======
 * Compute the positive gradient for t-SNE algorithm.
 * The gradient values are accumulated in the gradient array.
 * @param points: Array of current embedding points
 * @param gradient: Output array where the positive gradient will be accumulated
 * @param p_matrix: Sparse probability matrix P (high-dimensional similarities)
 * @param xaggerationFactor: Factor to scale attractive forces
 * @param debugRenderData: Data used for debugging or visualization
**/
void TSNEGradientCompute::computePositiveGradient(NDArray<vec>& points, NDArray<vec>& gradient, PMatrix& p_matrix, prec_float exaggerationFactor, DebugRenderData& debugRenderData)
{
	size_t numPoints = points.size();

	prec_float Z = q_denom_precalculated;

	float meanPositiveForceMagnitude = 0;

	// Implement the positive gradient calculation here



	meanPositiveForceMagnitude /= (float)numPoints;

	std::cout << "Mean positive force magnitude: " << meanPositiveForceMagnitude << std::endl;
}

/**
 ====== TODO: IMPLEMENT ======
 * Compute the negative gradient for t-SNE algorithm.
 * The gradient values are subtracted from the gradient array.
 * @param points: Array of current embedding points
 * @param gradient: Output array where the negative gradient will be subtracted
 * @param debugRenderData: Data used for debugging or visualization
**/
void TSNEGradientCompute::computeNegativeGradient(NDArray<vec>& points, NDArray<vec>& gradient, DebugRenderData& debugRenderData)
{
	size_t numPoints = points.size();

	prec_float Z = q_denom_precalculated;

	// Implement the negative gradient calculation here
}

void TSNEGradientCompute::setup(NDArray<vec>& points, DebugRenderData& debugRenderData)
{
	precalculate_q_denom(points, debugRenderData);
}

void TSNEGradientCompute::reset()
{
	q_denom_precalculated = 0.0f;
}

double TSNEGradientCompute::getTime()
{
	return glfwGetTime();
}

/**
 ====== TODO: IMPLEMENT ======
 * Precalculate the Q denominator for t-SNE algorithm.
 * The Q denominator is used to normalize the Q values.
 * @param points: Array of current embedding points
 * @param debugRenderData: Data used for debugging or visualization
**/
void TSNEGradientCompute::precalculate_q_denom(NDArray<vec>& points, DebugRenderData& debugRenderData)
{
	START_TIMER();

	size_t numPoints = points.size();

	//q_denom_precalculated = ...;

	LOG_TIME("Precalculating Q denominator");
}

prec_float TSNEGradientCompute::clamp(prec_float x, prec_float min, prec_float max)
{
	return glm::min(glm::max(x, min), max);
}

/**
 ====== TODO: IMPLEMENT ======
 * Compute the numerator of the q value
 */
prec_float TSNEGradientCompute::q_numerator(vec& a, vec& b)
{
	return 1.0;
}

prec_float TSNEGradientCompute::sqdistance(vec& a, vec& b)
{
	return static_cast<prec_float>(glm::distance2(a, b));
}

prec_float TSNEGradientCompute::q_denom(vec& a, vec& b)
{
	return 1.0f / (1.0f + sqdistance(a, b));
}

vec TSNEGradientCompute::negative_gradient(vec& a, vec& b)
{
	prec_float q_ij = q_numerator(a, b);

	return q_ij * q_ij * (b - a);
}