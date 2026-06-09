#include "tsne.h"

TSNE::TSNE()
{
}

TSNE::~TSNE()
{
}

void TSNE::step(TSNEGradientCompute& gradientComputer, DebugRenderData& debugRenderData)
{
	stepIndex++;
	std::cout << "\n====== Step " << stepIndex << " ======" << std::endl;
	computeGradient(gradientComputer, debugRenderData);


	std::cout << "Applying gradient" << std::endl;
	applyGradient();
}

unsigned int TSNE::getStepIndex() const
{
	return stepIndex;
}

float TSNE::getMeanDisplacement() const
{
	return (float)lastMeanDisplacement;
}

float TSNE::getSmoothedDisplacement() const
{
	return (float)smoothedDisplacement;
}

bool TSNE::isConverged(float threshold) const
{
	// can't be converged if nothing has run yet
	if (smoothedDisplacement < 0.0)
	{
		return false;
	}

	return smoothedDisplacement < threshold;
}

NDArray<vec>& TSNE::getPoints()
{
	return points;
}

bool TSNE::hasPoints() const
{
	return points.size() > 0;
}

void TSNE::setPoints(const NDArray<vec>& newPoints)
{
	this->points = newPoints;
    this->prev_points = points;
	this->gradient = NDArray<vec>::empty({ points.size() });
    this->stepIndex = 0;

    // new dataset, so drop the old movement history
    this->lastMeanDisplacement = -1.0;
    this->smoothedDisplacement = -1.0;
}

void TSNE::setPMatrix(const PMatrix& new_p_matrix)
{
    this->p_matrix = new_p_matrix;
}

void TSNE::setLabels(const NDArray<int>& newLabels)
{
	this->labels = newLabels;
}

NDArray<int>& TSNE::getLabels()
{
	return labels;
}

float TSNE::getAccuracy()
{
	if (!NearestNeighbourCalculator::isEnabled())
	{
		return 0.0f;
	}

	NDArray<unsigned int> highDimNearestNeighbours = NearestNeighbourCalculator::getNearestNeighbours();

	NDArray<float> pointsNDArray = NDArray<float>::empty({ (int)points.size(), 2 });

	for (int i = 0; i < points.size(); i++)
	{
		pointsNDArray(i, 0) = points(i).x;
		pointsNDArray(i, 1) = points(i).y;
	}

	NDArray<unsigned int> nearestNeighbours = NearestNeighbourCalculator::calculateNearestNeighbours(pointsNDArray);

	float accuracy = 0.0f;
	float totalOverlap = 0.0f;

	int numPoints = nearestNeighbours.shape[0];
	int numNeighbours = nearestNeighbours.shape[1];

	// Compute the accuracy with your metric
	for (int i = 0; i < numPoints; i++)
	{
		int matchCount = 0;

		// Compare the nearest neighbours in the high-dimensional space with those in the low-dimensional space
		for (int j = 0; j < numNeighbours; j++)
		{
			unsigned int highNeighbour = highDimNearestNeighbours(i, j);

			for (int k = 0; k < numNeighbours; k++)
			{
				unsigned int lowNeighbour = nearestNeighbours(i, k);

				if (highNeighbour == lowNeighbour)
				{
					// If there is a match, increment the match count and 
					// break out of the inner loop to avoid counting multiple matches for the same neighbour
					matchCount++;
					break;
				}
			}
			
		}
		// The overlap for this point is calculated as the ratio of the number of matching neighbours to the total number of neighbours, and this value is accumulated into the total overlap.
		totalOverlap += (float) matchCount / (float)numNeighbours;
	}

	// The final accuracy is computed as the average overlap across all points, which gives an indication of how well the low-dimensional embedding preserves the local structure of the high-dimensional data.
	accuracy = totalOverlap / (float)numPoints;
	std::cout << "Accuracy: " << accuracy << std::endl;

	return accuracy;
}

void TSNE::compareGradientComputers(TSNEGradientCompute& gradientComputer1, TSNEGradientCompute& gradientComputer2, DebugRenderData& debugRenderData)
{

	resetGradientValues();
	gradientComputer1.setup(this->points, debugRenderData);
	//gradientComputer1.computePositiveGradient(this->points, this->gradient, this->p_matrix, getExaggerationFactor(), debugRenderData);
	gradientComputer1.computeNegativeGradient(this->points, this->gradient, debugRenderData);

	NDArray<vec> gradient1 = this->gradient;

	resetGradientValues();
	gradientComputer2.setup(this->points, debugRenderData);
	//gradientComputer2.computePositiveGradient(this->points, this->gradient, this->p_matrix, getExaggerationFactor(), debugRenderData);
	gradientComputer2.computeNegativeGradient(this->points, this->gradient, debugRenderData);

	NDArray<vec> gradient2 = this->gradient;

	NDArray<vec> diff = NDArray<vec>::empty({ gradient1.shape[0] });

	prec_float maxDiffIndex = 0;

	prec_float meanDiffMagnitude = 0;
	prec_float meanGradient1Magnitude = 0;

	for (unsigned int i = 0; i < gradient1.size(); i++)
	{
		diff(i) = gradient1(i) - gradient2(i);
		prec_float diffMagnitude = glm::length(diff(i));
		prec_float gradient1Magnitude = glm::length(gradient1(i));

		meanDiffMagnitude += diffMagnitude;
		meanGradient1Magnitude += gradient1Magnitude;
	}

	meanDiffMagnitude /= gradient1.size();
	meanGradient1Magnitude /= gradient1.size();

	for (unsigned int i = 0; i < gradient1.size(); i++)
	{
		/*
		debugRenderData.addLine(points(i), points(i) + glm::normalize(-diff(i)) * 0.04, glm::vec3(1.0f, 0.0f, 0.0f));
		debugRenderData.addLine(points(i), points(i) + glm::normalize(gradient1(i)) * 0.04, glm::vec3(0.0f, 1.0f, 0.0f));
		debugRenderData.addLine(points(i), points(i) + glm::normalize(gradient2(i)) * 0.04, glm::vec3(0.0f, 0.0f, 1.0f));
		*/

		debugRenderData.addLine(points(i), points(i) + -diff(i) / meanGradient1Magnitude * 0.08, glm::vec3(1.0f, 0.0f, 0.0f));
		
		debugRenderData.addLine(points(i), points(i) + gradient1(i) / meanGradient1Magnitude * 0.08, glm::vec3(0.0f, 1.0f, 0.0f));
		debugRenderData.addLine(points(i), points(i) + gradient2(i) / meanGradient1Magnitude * 0.08, glm::vec3(0.0f, 0.0f, 1.0f));
	}

	prec_float error = meanDiffMagnitude / meanGradient1Magnitude;

	std::cout << "Mean diff magnitude: " << meanDiffMagnitude << " (" << error << ")" << std::endl;
}

void TSNE::computeGradient(TSNEGradientCompute& gradientComputer, DebugRenderData& debugRenderData)
{
	gradientComputer.setup(this->points, debugRenderData);

	resetGradientValues();
	gradientComputer.computeNegativeGradient(this->points, this->gradient, debugRenderData);
	gradientComputer.computePositiveGradient(this->points, this->gradient, this->p_matrix, getExaggerationFactor(), debugRenderData);
}	

void TSNE::applyGradient()
{
	NDArray<vec> old_points = points;

#pragma omp parallel for
	for (int i = 0; i < points.size(); i++)
	{
		if (stepIndex == 0)
		{
			points(i) += learning_rate * gradient(i);
		}
		else
		{
			points(i) += learning_rate * gradient(i) + momentum() * (points(i) - prev_points(i));
		}
	}

	prev_points = old_points;

	// work out how far the points moved this step on average
	double totalDisplacement = 0.0;
#pragma omp parallel for reduction(+:totalDisplacement)
	for (int i = 0; i < points.size(); i++)
	{
		totalDisplacement += glm::length(points(i) - old_points(i));
	}

	lastMeanDisplacement = totalDisplacement /(double)points.size();

	// keep a smoothed average so the converged label doesn't bounce around
	if (smoothedDisplacement < 0.0)
	{
		smoothedDisplacement = lastMeanDisplacement; // first step
	}
	else
	{
		smoothedDisplacement = 0.9 * smoothedDisplacement + 0.1 * lastMeanDisplacement;
	}
}

void TSNE::resetGradientValues()
{
	for (unsigned int i = 0; i < gradient.size(); i++)
	{
		gradient(i) = vec(0.0f);
	}
}

prec_float TSNE::momentum()
{
	if (stepIndex < 250)
	{
		return 0.5f;
	}

	return 0.8f;
}

prec_float TSNE::getExaggerationFactor()
{
	if (stepIndex < early_exaggeration_steps)
	{
		return early_exaggeration;
	}

	return 1.0f;
}