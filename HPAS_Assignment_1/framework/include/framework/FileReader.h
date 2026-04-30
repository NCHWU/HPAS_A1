#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>

#include "framework/SavedSettings.h"

struct Column
{
	std::string name;
	std::vector<float> data;
};

namespace FileReader
{
	std::vector<Column> readCSV(const char* filePath);

	std::vector<int> readLabels(const char* filePath, unsigned int sampleFrequency = 1);

	const char* getCurrentFile();
};