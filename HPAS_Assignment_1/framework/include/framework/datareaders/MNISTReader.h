#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring> // for memset
#ifdef WIN32 // for ntohl
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include "DataReader.h"
#include "NearestNeighbourCalculator.h"

class MNISTReader : public DataReader
{
public:
	MNISTReader();
	~MNISTReader();

	void readData(TSNE& tsne, SquareDistanceCalculator& squareDistanceCalculator, Settings& settings, int numNeighbours, bool bigSet = false);

	unsigned int randi(int max)
	{
		return std::rand() % max;
	}

private:

    // Function to read the 4-byte integers from the file in MSB-first format
	int readInt(std::ifstream& file);

    // Function to read MNIST image data into a float array
    float* readImages(const std::string& filename, int& numImages, int& numRows, int& numCols);

    // Function to read MNIST label data into an integer array
	std::vector<int> readLabels(const std::string& filename, int& numLabels);
};