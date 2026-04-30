#pragma once

#include <string>

#include "DataReader.h"

class PlanariaReader : public DataReader
{
public:
	PlanariaReader();
	~PlanariaReader();

	void readData(TSNE& tsne, SquareDistanceCalculator& squareDistanceCalculator, Settings& settings, int numNeighbours);

	unsigned int randi(int max)
	{
		return std::rand() % max;
	}
};