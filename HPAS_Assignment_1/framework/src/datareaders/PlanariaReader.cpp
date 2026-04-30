#include "datareaders/PlanariaReader.h"
#include <filesystem>
#include <iostream>

PlanariaReader::PlanariaReader()
{

}

PlanariaReader::~PlanariaReader()
{

}

void PlanariaReader::readData(TSNE& tsne, SquareDistanceCalculator& squareDistanceCalculator, Settings& settings, int numNeighbours)
{
	unsigned int numPoints = 21612;
	std::vector<float> p_matrix = std::vector<float>(numPoints * numPoints);
	std::vector<float> p_matrix_symm = std::vector<float>(numPoints * numPoints);

	float perplexity = (float)numNeighbours;
	float sigma = 1.0f;

	unsigned int numColumns = 54;
	float* rawData = (float*)malloc(numPoints * numColumns * sizeof(float));

	if (rawData == nullptr)
	{
		std::cerr << "Failed to allocate memory for the raw data" << std::endl;
		return;
	}
	std::ifstream file(RESOURCE_ROOT "../resources/datasets/planaria/R_pca_seurat.txt");

	if (!file.is_open())
	{
		std::cerr << "Error: Could not open data file for Planaria" << std::endl;
		return;
	}

	std::string line;

	// Read the data
	unsigned int index = 0;
	while (std::getline(file, line))
	{
		std::stringstream lineStream(line);
		std::string value;

		// Parse each value in the line
		while (std::getline(lineStream, value, ' '))
		{
			float fval = std::stof(value);

			rawData[index] = fval;

			index++;
		}
	}

	file.close();


	std::cout << "Calculating P matrix" << std::endl;
	PMatrixCalculator pMatrixCalculator;
	PMatrix P = pMatrixCalculator.getPMatrix(rawData, static_cast<int>(numPoints), numColumns, perplexity, squareDistanceCalculator, "Planaria");

	NDArray<float> data = NDArray<float>::empty({ (size_t)numPoints, (size_t)numColumns });

	for (unsigned int i = 0; i < numPoints; i++)
	{
		for (unsigned int j = 0; j < numColumns; j++)
		{
			data(i, j) = rawData[i * numColumns + j];
		}
	}

	NearestNeighbourCalculator::calculateNearestNeighbours("planaria", data, 30);

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

	std::vector<int> labels = FileReader::readLabels(RESOURCE_ROOT "../resources/datasets/planaria/R_annotation.txt");

	tsne.setLabels(labels);

	return;
}
