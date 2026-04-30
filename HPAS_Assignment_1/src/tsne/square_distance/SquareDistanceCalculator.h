#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <string>
#include <GLFW/glfw3.h>
#include <omp.h>

#include <framework/NDArray.h>

struct ValueAndIndex
{
	float value = 0.0f;
	unsigned int index = 0;
	ValueAndIndex(float v = 0.0f, unsigned int index = 0)
		: value(v), index(index)
	{}
};

template <typename T>
struct Array2D
{
	T* data{ nullptr };

	int numRows{ 0 };
	int numCols{ 0 };

	Array2D(int numRows, int numCols, bool doAllocation = true)
	{
		this->numRows = numRows;
		this->numCols = numCols;
		this->data = nullptr;
		if (doAllocation)
		{
			allocateMemory();
		}
		std::cout << "Creating Array2D (" << numCols << " x " << numRows << ")" << std::endl;
	}

	Array2D()
	{}

	~Array2D()
	{
		std::cout << "Destroying Array2D (" << numCols << " x " << numRows << ")" << std::endl;
	}

	void freeMemory()
	{
		free(data);
		data = nullptr;
		std::cout << "Freeing Array2D (" << numCols << " x " << numRows << ")" << std::endl;
	}

	void allocateMemory()
	{
		data = (T*)malloc(numRows * numCols * sizeof(T));
	}

	T sample(int row, int col)
	{
		if (col < 0 || col >= numCols || row < 0 || row >= numRows)
		{
			std::cerr << "Sampling data outside of possible range in Array2D (" << numCols << " x " << numRows << ")"
				<< " at (row: " << row << ", col: " << col << ")"
				<< std::endl;
		}

		int index = col + row * numCols;
		return data[index];
	}

	void set(int row, int col, T value)
	{
		if (col < 0 || col >= numCols || row < 0 || row >= numRows)
		{
			std::cerr << "Setting data outside of possible range in Array2D (" << numCols << " x " << numRows << ")"
				<< " at (row: " << row << ", col: " << col << ")"
				<< std::endl;
		}

		int index = col + row * numCols;
		data[index] = value;
	}
};

class SquareDistanceCalculator
{
public:
	virtual NDArray<ValueAndIndex> getSquareDistances(float* data, int numRows, int numCols, int numNeighbours) = 0;

	float getTime()
	{
		return static_cast<float>(glfwGetTime());
	}
};