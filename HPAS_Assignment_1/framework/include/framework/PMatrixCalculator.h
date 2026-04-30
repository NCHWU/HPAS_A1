#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <string>

#include <fstream>
#include <iostream>
#include <iostream>
#include <iomanip>

#include "shaders/ComputeShader.h"
#include "framework/WindowHandler.h"
#include "../../src/tsne/square_distance/SquareDistanceCalculator.h"
#include <GLFW/glfw3.h>
#include <omp.h>
#include "framework/NDArray.h"

#define PMatrix NDArray<ValueAndIndex>

template <typename T>
struct HighDimData
{
	T* data;

	int numRows{ 0 };
	int numCols{ 0 };

	HighDimData(int numRows, int numCols)
	{
		this->numRows = numRows;
		this->numCols = numCols;

		data = (T*)malloc(numRows * numCols * sizeof(T));
		std::cout << "Creating HighDimData" << std::endl;
	}

	~HighDimData()
	{
		free(data);
		data = nullptr;
		std::cout << "Freeing HighDimData" << std::endl;
	}

	T* sample(int index)
	{
		if (index < 0 || index >= this->numRows)
		{
			std::cerr << "Sampling data outside of possible range in HighDimData" << std::endl;
		}

		return data + index * numCols;
	}
};

class PMatrixCalculator
{
public:
	PMatrixCalculator();
	~PMatrixCalculator();

	/// <summary>
	/// Get the P matrix for the given data. Attempts to load the P matrix from cache if it exists.
	/// If the cache does not exist, it will be calculated and saved to the cache.
	/// The implementation used for this calculation can be chosen within the class.
	/// </summary>
	/// <param name="data">The high-dimensional data to create the P matrix for</param>
	/// <param name="numRows">The number of rows in the high-dimensional data</param>
	/// <param name="numCols">The number of columns in the high-dimensional data</param>
	/// <param name="perplexity">The desired perplexity for the P matrix</param>
	/// <param name="cacheIdentifier">The identifier used to find this data in the cache. Keep consistent for the cache to function correctly</param>
	/// <returns></returns>
	NDArray<ValueAndIndex> getPMatrix(float* data, int numRows, int numCols, float perplexity, SquareDistanceCalculator& squareDistanceCalculator, const char* cacheIdentifier = "");

private:

	float squareDistance(float* a, float* b);

	int getNumNeighbours(int num_samples, float perplexity);

	NDArray<ValueAndIndex> getSquareDistances(float* data, int numNeighbours);

	NDArray<ValueAndIndex> binarySearchPerplexity(NDArray<ValueAndIndex> sqDistances, float perplexity);

	float getTime();

	int numRows{ 0 };
	int numCols{ 0 };
	int numNeighbours{ 0 };
};