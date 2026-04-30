#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <iostream>
#include <iomanip>

#include "framework/NDArray.h"

class NearestNeighbourCalculator
{
public:

	// Calculate the nearest neighbours for each point in the data set
	// This is performed by the data reader and should not be called by the student
	static void calculateNearestNeighbours(const char* identifier, NDArray<float>& points, unsigned int numNeighbours);

	static NDArray<unsigned int> calculateNearestNeighbours(NDArray<float>& points);

	// Get the nearest neighbours for each point in the data set
	static NDArray<unsigned int>& getNearestNeighbours();

	static NearestNeighbourCalculator* getInstance()
	{
		// Create instance if it doesn't exist yet
		if (instance == nullptr)
		{
			instance = new NearestNeighbourCalculator();
		}
		return instance;
	}

	static void enable()
	{
		getInstance()->enabled = true;
	}

	static bool isEnabled()
	{
		return getInstance()->enabled;
	}

private:
	NearestNeighbourCalculator();
	~NearestNeighbourCalculator();
	static NearestNeighbourCalculator* instance;

	NDArray<unsigned int> nearestNeighbours;
	unsigned int numNeighbours;
	bool enabled{ false };
};