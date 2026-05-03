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

	// pmatrix structure is a value and index pair for each point and its nearest neighbours. 
	// The value represents the high-dimensional similarity (p_ij) between the points, while the index indicates the position of the neighbor point in the original dataset.
	int num_neighbours = p_matrix.shape[1];

	// Implement the positive gradient calculation here
	for (size_t i = 0; i < numPoints; i++)
	{
		for (size_t j = 0; j < num_neighbours; j++)
		{
			// The index of the neighbor point is obtained from the p_matrix, which contains the indices of the nearest neighbors for each point.
			int neighbourIndex = p_matrix(i, j).index;

			if (i != neighbourIndex)
			{			

				// The q value for the pair of points (i, neighborIndex) is calculated using the q_numerator function, which computes the numerator of the q value based on the distance between the two points in the low-dimensional space.
				prec_float q_ij = q_numerator(points(i), points(neighbourIndex));

				// The positive gradient is calculated using the formula:p_ij * q_ij * (y_j - y_i)

				vec posGrad = p_matrix(i, j).value * q_ij * (points(neighbourIndex) - points(i));

				// The positive gradient is scaled by the exaggeration factor to increase the attractive forces during the early iterations of the t-SNE algorithm.
				posGrad *= exaggerationFactor;

				// Scaling factor to balance the attractive and repulsive forces in the t-SNE algorithm.
				posGrad *= 4; 

				// The calculated positive gradient is accumulated in the gradient array for each point i.
				gradient(i) += posGrad;

				// For debugging or visualization purposes, the magnitude of the positive forces can be calculated and printed to understand the behavior of the algorithm.
				meanPositiveForceMagnitude += glm::length(posGrad);
			}
		}
	}

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

// The negative gradient is calculated by iterating over all pairs of points and computing the contribution of each pair 
// to the negative gradient. For each pair of points i and j, the negative gradient is computed using the formula:
// negative_gradient = q_ij^2 * (y_j - y_i) / Z
// Negative gradient is gradient of the Kullback-Leibler divergence with respect to the low-dimensional embedding points.
void TSNEGradientCompute::computeNegativeGradient(NDArray<vec>& points, NDArray<vec>& gradient, DebugRenderData& debugRenderData)
{
	size_t numPoints = points.size();

	prec_float Z = q_denom_precalculated;

	float meanNegativeForceMagnitude = 0.0f;

	// Implement the negative gradient calculation here

	for (size_t i = 0; i < numPoints; i++)
	{
		for (size_t j = 0; j < numPoints; j++)
		{
			if (i != j)
			{
				// Negative gradient is calculated using the q_numerator function to compute the q value for the pair of points (i, j), 
				// and then applying the formula for the negative gradient.
				vec negGrad = negative_gradient(points(i), points(j));
				negGrad /= Z;

				// The negative gradient is scaled by a factor of 4 to balance the attractive and repulsive forces in the t-SNE algorithm.
				negGrad *= 4;

				// The calculated negative gradient is subtracted from the gradient array for each point i.
				gradient(i) -= negGrad;

				meanNegativeForceMagnitude += glm::length(negGrad);
			}
		}
	}

	meanNegativeForceMagnitude /= (float)numPoints;

	std::cout << "Mean negative force magnitude: " << meanNegativeForceMagnitude << std::endl;
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

// The Q denominator is defined as the sum of 1 / (1 + ||y_i - y_j||^2) for all pairs of points i and j, 
// where y_i and y_j are the low-dimensional embeddings of points i and j, respectively. 
void TSNEGradientCompute::precalculate_q_denom(NDArray<vec>& points, DebugRenderData& debugRenderData)
{
	START_TIMER();

	size_t numPoints = points.size();
	reset();

	//Looping over all pairs of points to calculate the Q denominator. 
	for (size_t i = 0; i < numPoints; i++)
	{
		for (size_t j = 0; j < numPoints; j++)
		{
			if (i != j)
			{
				q_denom_precalculated += q_denom(points(i), points(j));
			}
		}
	}
	LOG_TIME("Precalculating Q denominator");
	std::cout << "Q denom: " << q_denom_precalculated << std::endl;

}

prec_float TSNEGradientCompute::clamp(prec_float x, prec_float min, prec_float max)
{
	return glm::min(glm::max(x, min), max);
}

/**
 ====== TODO: IMPLEMENT ======
 * Compute the numerator of the q value
 */
 // q_numerator is defined as 1 / (1 + ||y_i - y_j||^2) where y_i and y_j are the low-dimensional embeddings of points 
 // i and j, respectively. This function calculates the numerator of the q value for two given points a and b.
prec_float TSNEGradientCompute::q_numerator(vec& a, vec& b)
{
	return 1.0f / (1.0f + sqdistance(a, b));
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