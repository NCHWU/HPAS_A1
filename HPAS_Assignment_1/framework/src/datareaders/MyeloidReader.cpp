#include "datareaders/MyeloidReader.h"
#include <filesystem>
#include <iostream>

MyeloidReader::MyeloidReader()
{

}

MyeloidReader::~MyeloidReader()
{

}

void MyeloidReader::readData(TSNE& tsne, SquareDistanceCalculator& squareDistanceCalculator, Settings& settings, int numNeighbours)
{
	unsigned int numPoints;
	unsigned int numColumns;

	// Read the raw data

	float* rawData = nullptr;
	
	try
	{
		rawData = readRawData(numPoints, numColumns);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error reading Myeloid data: " << e.what() << std::endl;
		return;
	}

	NDArray<float> data = NDArray<float>::empty({ (size_t)numPoints, (size_t)numColumns });

	for (unsigned int i = 0; i < numPoints; i++)
	{
		for (unsigned int j = 0; j < numColumns; j++)
		{
			data(i, j) = rawData[i * numColumns + j];
		}
	}

	NearestNeighbourCalculator::calculateNearestNeighbours("Myeloid", data, 30);

	std::vector<float> p_matrix = std::vector<float>(numPoints * numPoints);
	std::vector<float> p_matrix_symm = std::vector<float>(numPoints * numPoints);

	float perplexity = (float)numNeighbours;
	float sigma = 1.0f;


	std::cout << "Calculating P matrix" << std::endl;
	PMatrixCalculator pMatrixCalculator;
	PMatrix P = pMatrixCalculator.getPMatrix(rawData, static_cast<int>(numPoints), numColumns, perplexity, squareDistanceCalculator, "Myeloid");

	NDArray<vec> points;
	if (settings.usePCA)
		points = PCA::embed(data);
	else
		points = randomEmbed(data.shape[0]);


	// Normalizing the point locations to be in [0,1]
	prec_float minX = points(0).x;
	prec_float minY = points(0).y;
	prec_float maxX = points(0).x;
	prec_float maxY = points(0).y;

	for (unsigned int i = 0; i < numPoints; i++)
	{
		// Updating the boundaries
		minX = glm::min(minX, points(i).x);
		minY = glm::min(minY, points(i).y);
		maxX = glm::max(maxX, points(i).x);
		maxY = glm::max(maxY, points(i).y);
	}

	for (unsigned int i = 0; i < numPoints; i++)
	{
		points(i).x = (points(i).x - minX) / (maxX - minX);
		points(i).y = (points(i).y - minY) / (maxY - minY);
	}


	tsne.setPMatrix(P);
	tsne.setPoints(points);

	std::vector<int> labels = FileReader::readLabels(RESOURCE_ROOT "../resources/datasets/myeloid8000/myeloid_8000_labels.csv", 1);

	tsne.setLabels(labels);

	return;
}

float* MyeloidReader::readRawData(unsigned int& numPoints, unsigned int& numColumns)
{
	unsigned int sampleFrequency = 1;
	numPoints = ((8000 + sampleFrequency - 1) / sampleFrequency);
	numColumns = 11;

	float* rawData = (float*)malloc(numPoints * numColumns * sizeof(float));

	if (rawData == nullptr)
	{
		// std::exception with a const char* is an MSVC-only ordeal!!
		throw std::runtime_error("Failed to allocate memory for the raw data");
		return nullptr;
	}
	std::ifstream file(RESOURCE_ROOT "../resources/datasets/myeloid8000/myeloid_8000.csv");

	if (!file.is_open())
	{
		throw std::runtime_error("Could not open CSV file for Myeloid");
		return nullptr;
	}

	std::string line;

	// Read the header (first line with column names)
	if (std::getline(file, line))
	{
		std::stringstream headerStream(line);
		std::string columnName;

		// Initialize columns with names
		while (std::getline(headerStream, columnName, ','))
		{

		}
	}

	// Read the data
	unsigned int index = 0;
	unsigned int dataIndex = 0;
	while (std::getline(file, line))
	{
		std::stringstream lineStream(line);
		std::string value;

		// Parse each value in the line
		while (std::getline(lineStream, value, ','))
		{
			float fval = std::stof(value);

			if (index % sampleFrequency == 0) 
				rawData[dataIndex++] = fval;

			index++;
		}
	}

	file.close();

	return rawData;
}

void MyeloidReader::shuffle(std::vector<Column>& columns, unsigned int count)
{
	for (unsigned int i = 0; i < count; i++)
	{
		unsigned int j = randi(count);
		// Swap
		for (unsigned int k = 0; k < columns.size(); k++)
		{
			float temp = columns[k].data[i];
			columns[k].data[i] = columns[k].data[j];
			columns[k].data[j] = temp;
		}
	}
}
