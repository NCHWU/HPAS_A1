#pragma once

#include <string>

#include "DataReader.h"

class MyeloidReader : public DataReader
{
public:
	MyeloidReader();
	~MyeloidReader();

	void readData(TSNE& tsne, SquareDistanceCalculator& squareDistanceCalculator, Settings& settings, int numNeighbours);

	// Read the raw myeloid data. Don't forget to free the memory when you're done with it!
	float* readRawData(unsigned int& numPoints, unsigned int& numColumns);

	void shuffle(std::vector<Column>& columns, unsigned int count);

	unsigned int randi(int max)
	{
		return std::rand() % max;
	}
};