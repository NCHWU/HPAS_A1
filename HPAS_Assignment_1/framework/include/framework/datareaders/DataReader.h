#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "framework/FileReader.h"
#include "../../src/tsne/tsne.h"
#include "../../src/tsne/PCA.h"
#include "../../src/Settings.h"
#include "../../src/tsne/square_distance/SquareDistanceCalculator.h"
#include "framework/PMatrixCalculator.h"
#include "framework/PMatrixCalculator.h"
#include "framework/NDArray.h"

struct DataPoint
{
	glm::vec2 pos;
	int label;
};

struct Data
{
	std::vector<DataPoint> points;
};

class DataReader
{
public:
	DataReader() = default;
	virtual ~DataReader() = default;

	NDArray<vec> randomEmbed(size_t count)
	{
		NDArray<vec> data = NDArray<vec>::empty({ count });
		srand(0);
		for (size_t i = 0; i < count; ++i)
		{
			data(i).x = rand();
			data(i).y = rand();
		}
		return data;
	}

protected:
	float rand()
	{
		return static_cast<float>(std::rand()) / RAND_MAX;
	}
};