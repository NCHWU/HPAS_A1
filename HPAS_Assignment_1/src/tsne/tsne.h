#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <omp.h>
#include <iostream>
#include "framework/PMatrixCalculator.h"
#include "framework/datareaders/NearestNeighbourCalculator.h"
#include "framework/DebugRenderData.h"

#include "GradientComputers/TSNEGradientCompute.h"

#define TSNE_PRECISION 2

typedef DebugRenderData2D DebugRenderData;

#if TSNE_PRECISION == 1
typedef float prec_float;
typedef glm::vec2 vec;
#elif TSNE_PRECISION == 2
typedef double prec_float;
typedef glm::dvec2 vec;
#endif

struct GradientComparisonResult
{
	prec_float meanDiffMagnitude{ 0.0 };
	prec_float meanReferenceGradientMagnitude{ 0.0 };
	prec_float relativeError{ 0.0 };
};

class TSNE
{
public:
	TSNE();

	virtual ~TSNE();

	virtual void step(TSNEGradientCompute& gradientComputer, DebugRenderData& debugRenderData);

	// Get the index of the current t-SNE step
	unsigned int getStepIndex() const;

	// how far the points moved on average last step (-1 if we haven't stepped yet)
	float getMeanDisplacement() const;

	// same value but smoothed over time, this is what we check for convergence
	float getSmoothedDisplacement() const;

	// true once the points have basically stopped moving (below threshold)
	bool isConverged(float threshold) const;

	// Get all points currently loaded into this t-SNE instance
	NDArray<vec>& getPoints();

	// Set the points to be used in the t-SNE algorithm
	void setPoints(const NDArray<vec>& newPoints);

	// Get whether or not this t-SNE instance has any points
	bool hasPoints() const;

	// Set the P matrix to be used in the t-SNE algorithm
	// This is performed by the data reader and should not be called by the student
	void setPMatrix(const PMatrix& new_p_matrix);

	// Set the labels to be used in the t-SNE algorithm
	// These determine the colours used when rendering the points
	// This is performed by the data reader and should not be called by the student
	void setLabels(const NDArray<int>& newLabels);

	// Get the labels currently loaded into this t-SNE instance
	NDArray<int>& getLabels();

	float getAccuracy();

	GradientComparisonResult compareGradientComputers(TSNEGradientCompute& gradientComputer1, TSNEGradientCompute& gradientComputer2, DebugRenderData& debugRenderData);

protected:

	// Compute the full gradient of the t-SNE algorithm
	virtual void computeGradient(TSNEGradientCompute& gradientComputer, DebugRenderData& debugRenderData);

	// Apply the gradient to the points
	// This moves the points in the direction of the gradient
	void applyGradient();

	// Reset all values in the gradient vector to 0
	void resetGradientValues();
	
	// Get the momentum value for the current step
	prec_float momentum();

	prec_float getExaggerationFactor();

	NDArray<vec> points;
	NDArray<vec> prev_points;
	PMatrix p_matrix;
	NDArray<vec> gradient;
	NDArray<int> labels;

	// Parameters for t-SNE
	prec_float learning_rate{ 100.0f };
	prec_float early_exaggeration{ 3.0f };
	unsigned int early_exaggeration_steps{ 2500 };
	unsigned int stepIndex{ 0 };

	// how much the points moved last step (-1 = no step yet)
	prec_float lastMeanDisplacement{ -1.0 };
	// smoothed version of the above so the converged label doesn't flicker
	prec_float smoothedDisplacement{ -1.0 };
};
