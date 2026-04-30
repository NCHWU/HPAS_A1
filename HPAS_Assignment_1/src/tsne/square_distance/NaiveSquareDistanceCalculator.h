#pragma once

#include "SquareDistanceCalculator.h"

class NaiveSquareDistanceCalculator : public SquareDistanceCalculator
{
public:
	NaiveSquareDistanceCalculator();
	~NaiveSquareDistanceCalculator();

	virtual NDArray<ValueAndIndex> getSquareDistances(float* data, int numRows, int numCols, int numNeighbours) override;
};
