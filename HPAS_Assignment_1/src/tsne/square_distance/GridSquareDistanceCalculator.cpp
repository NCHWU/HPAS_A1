#include "GridSquareDistanceCalculator.h"

GridSquareDistanceCalculator::GridSquareDistanceCalculator()
{}

GridSquareDistanceCalculator::~GridSquareDistanceCalculator()
{}

NDArray<ValueAndIndex> GridSquareDistanceCalculator::getSquareDistances(float* data, int numRows, int numCols, int numNeighbours)
{
    NDArray<ValueAndIndex> sqDistances = NDArray<ValueAndIndex>::empty({ numRows, numNeighbours });

    float startTime = getTime();

#pragma omp parallel for
    for (int i = 0; i < numRows; i++)
    {
        std::vector<ValueAndIndex> allDistances(numRows);

        // Calculate all distances to this point
        for (int j = 0; j < numRows; j++)
        {
            // Ignore distance to itself
            if (i == j)
            {
                allDistances[j] = ValueAndIndex(std::numeric_limits<float>::infinity(), j); // Ignore distance to itself
                continue;
            }

            float sqDistance = 0.0f;

            // Calculate squared Euclidean distance
            for (int k = 0; k < numCols; k++)
            {
                float diff = data[i * numCols + k] - data[j * numCols + k];
                sqDistance += diff * diff;
            }

            allDistances[j] = ValueAndIndex(sqDistance, j);
        }

        // Iteratively selecting the n closest neighbours
        for (int j = 0; j < numNeighbours; j++)
        {
            ValueAndIndex lowestDistance = allDistances[0];
            int lowestIndex = 0;

            // Finding the current lowest distance
            for (int k = 1; k < numRows; k++)
            {
                if (allDistances[k].value <= lowestDistance.value)
                {
                    lowestDistance = allDistances[k];
                    lowestIndex = k;
                }
            }

            // Selecting the lowest distance
            sqDistances(i, j) = lowestDistance;

            // Setting the lowest distance to infinity to avoid selecting it again
            allDistances[lowestIndex].value = std::numeric_limits<float>::infinity();
        }
    }

    float endTime = getTime();
    std::cout << "Time taken (naive): " << endTime - startTime << " seconds" << std::endl;
    return sqDistances;
}
