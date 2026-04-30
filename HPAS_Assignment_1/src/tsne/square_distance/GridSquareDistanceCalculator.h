#pragma once

#include "SquareDistanceCalculator.h"



class GridSquareDistanceCalculator : public SquareDistanceCalculator
{
public:
	GridSquareDistanceCalculator();
	~GridSquareDistanceCalculator();

	virtual NDArray<ValueAndIndex> getSquareDistances(float* data, int numRows, int numCols, int numNeighbours) override;
};
