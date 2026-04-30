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

	// Compute the accuracy with your metric

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