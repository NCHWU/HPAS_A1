#include "datareaders/NearestNeighbourCalculator.h"

NearestNeighbourCalculator* NearestNeighbourCalculator::instance = nullptr;


NearestNeighbourCalculator::NearestNeighbourCalculator()
{
}

NearestNeighbourCalculator::~NearestNeighbourCalculator()
{}

void NearestNeighbourCalculator::calculateNearestNeighbours(const char* identifier, NDArray<float>&points, unsigned int numNeighbours)
{
	if (!getInstance()->enabled)
	{
		return;
	}

	getInstance()->numNeighbours = numNeighbours;

	// Attempt to load the nearest neighbours from cache
	std::string cachePath = RESOURCE_ROOT "../resources/cache/" + std::string(identifier) + "_nearest_neighbours_" + std::to_string(numNeighbours) + ".bin";
    std::ifstream cacheFile(cachePath, std::ios::binary | std::ios::in);
    if (cacheFile.is_open())
    {
        // load cached file
        std::cout << "Nearest neighbours for " << identifier << " at " << numNeighbours << " neighbours already exists" << std::endl;

		// Read the nearest neighbours from cache

		NDArray<unsigned int> nearestNeighboursList = NDArray<unsigned int>::empty({ (unsigned int)points.shape[0], numNeighbours });

		// Reading the binary data and storing it in the P matrix
		cacheFile.read(reinterpret_cast<char*>(nearestNeighboursList.begin()), nearestNeighboursList.size() * sizeof(unsigned int));

		if (!cacheFile)
		{
			std::cerr << "Error: Failed to read complete data from cache file." << std::endl;
			return;
		}

		cacheFile.close();

		getInstance()->nearestNeighbours = nearestNeighboursList;
		getInstance()->numNeighbours = numNeighbours;

        return;
    }
    std::cout << "Calculating nearest neighbours for " << identifier << " at " << numNeighbours << " neighbours" << std::endl;

	NDArray<unsigned int> nearestNeighboursList = calculateNearestNeighbours(points);

	getInstance()->nearestNeighbours = nearestNeighboursList;
	getInstance()->numNeighbours = numNeighbours;

	// Save the nearest neighbours to cache
	std::ofstream cacheFileOut(cachePath, std::ios::binary | std::ios::out);
	if (cacheFileOut.is_open())
	{
		cacheFileOut.write(reinterpret_cast<const char*>(nearestNeighboursList.begin()), nearestNeighboursList.size() * sizeof(unsigned int));
		cacheFileOut.close();
	}
	else
	{
		std::cerr << "Failed to save nearest neighbours to cache" << std::endl;
	}
}


/*
 Finds and returns the nearest neighbours for each point in the data set.
 * points: the points to find the nearest neighbours for.
 Returns an array of size (numPoints, numNeighbours) containing the indices 
 of the nearest neighbours for each point.
*/
NDArray<unsigned int> NearestNeighbourCalculator::calculateNearestNeighbours(NDArray<float>& points)
{
	NDArray<unsigned int> nearestNeighboursList
		= NDArray<unsigned int>::empty({ (unsigned int)points.shape[0], getInstance()->numNeighbours });

	// Calculate the nearest neighbours
#pragma omp parallel for
	for (int i = 0; i < points.shape[0]; i++)
	{
		// Calculate the distance to each point
        NDArray<double> distances = NDArray<double>::empty({ points.shape[0] });

		for (size_t j = 0; j < points.shape[0]; j++)
		{
			distances(j) = 0.0f;
			for (size_t k = 0; k < points.shape[1]; k++)
			{
				double diff = points(i, k) - points(j, k);
				distances(j) += diff * diff;
			}
		}



		for (size_t j = 0; j < getInstance()->numNeighbours; j++)
		{
			unsigned int minIndex = 0;
            double nearestDistance = std::numeric_limits<double>::max();
			for (size_t k = 0; k < points.shape[0]; k++)
			{
				if (k == i)
				{
					continue;
				}
                if (distances(k) < nearestDistance && distances(k) > 0.0)
				{
                    nearestDistance = distances(k);
					minIndex = k;
				}
			}
			nearestNeighboursList(i, j) = minIndex;
            distances(minIndex) = std::numeric_limits<double>::max();
		}
	}

	return nearestNeighboursList;
}

NDArray<unsigned int>& NearestNeighbourCalculator::getNearestNeighbours()
{
	return getInstance()->nearestNeighbours;
}
