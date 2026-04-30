#pragma once

#include "framework/shaders/ComputeShader.h"

#include "SquareDistanceCalculator.h"
#include "framework/WindowHandler.h"

class GPUSquareDistanceCalculator : public SquareDistanceCalculator
{
public:
	GPUSquareDistanceCalculator();
	~GPUSquareDistanceCalculator();

	virtual NDArray<ValueAndIndex> getSquareDistances(float* data, int numRows, int numCols, int numNeighbours) override;
};
