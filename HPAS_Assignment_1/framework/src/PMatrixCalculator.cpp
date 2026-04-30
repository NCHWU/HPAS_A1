#include "PMatrixCalculator.h"

PMatrixCalculator::PMatrixCalculator()
{
}

PMatrixCalculator::~PMatrixCalculator()
{
}

NDArray<ValueAndIndex> PMatrixCalculator::getPMatrix(float* data, int numRows, int numCols, float perplexity, SquareDistanceCalculator& squareDistanceCalculator, const char* cacheIdentifier)
{
    if (perplexity < 0.0f)
    {
        std::cerr << "Invalid perplexity value: " << perplexity << std::endl;
    }

    this->numRows = numRows;
    this->numCols = numCols;
    this->numNeighbours = getNumNeighbours(numRows, perplexity);


    // Attempt to load the P matrix from cache
    std::string cachePath
        = RESOURCE_ROOT "../resources/cache/" + std::string(cacheIdentifier) + "_" 
        + std::to_string(numRows) + "rows_"
        + std::to_string(numCols) + "cols_"
        + std::to_string(int(perplexity)) + "perp"
        + "_P_matrix";
    std::ifstream cacheFile(cachePath, std::ios::binary | std::ios::in);
    if (cacheFile.is_open())
    {
        // load cached file
        std::cout << "Loading cached P matrix: " << cachePath << std::endl;

        size_t numElements = numRows * numNeighbours;

        NDArray<ValueAndIndex> P = NDArray<ValueAndIndex>::empty({ numRows, numNeighbours });

        // Reading the binary data and storing it in the P matrix
        cacheFile.read(reinterpret_cast<char*>(P.begin()), numElements * sizeof(ValueAndIndex));

        if (!cacheFile)
        {
            std::cerr << "Error: Failed to read complete data from cache file." << std::endl;
        }
        else
        {
        	return P;
        }
    }

    // Perform k-NN to find the N nearest neighbours
    
    std::cout << "Determining the distances between each point" << std::endl;
    NDArray<ValueAndIndex> distances = squareDistanceCalculator.getSquareDistances(data, numRows, numCols, numNeighbours);


    // Binary search complexity!!
    std::cout << "Performing binary search on the perplexity value for the P matrix" << std::endl;
    NDArray<ValueAndIndex> P = binarySearchPerplexity(distances, perplexity);

    //distances.freeMemory();

    // Enforcing symmetry on the P matrix

    std::cout << "Symmetrizing the P matrix" << std::endl;
    if (true) {
        for (int i = 0; i < numRows; i++) {
            // Iterate over the nearest neighbors of the current row
            for (int j = 0; j < numNeighbours; j++) {
                int index_j = P(i, j).index; // Get the index of the j-th neighbor of i
                float value = P(i, j).value; // Get the value of the affinity

                //std::cout << "Checking index " << j << "(" << index_j << ") at row " << i << std::endl; 

                for (int k = 0; k < numNeighbours; k++) {
                    if (P(index_j, k).index == i) {
                        // Symmetrical sample found!
                        float avg = (P(j, k).value + value) / 2.0f;
                        //std::cout << "Setting (" << i << ", " << index_j << ") and (" << index_j << ", " << i << ") to " << avg << std::endl;
                        P(i, j) = ValueAndIndex(avg, index_j);
                        P(index_j, k) = ValueAndIndex(avg, i);
                        break;
                    }
                }
            }
        }
    }
    
    bool calculateFullP = false;

    if (calculateFullP) {

        std::cout << "Writing P matrix to file for debug purposes" << std::endl;
        float* full_P = (float*)calloc(numRows * numRows, sizeof(float));
        if (full_P == nullptr) {
            std::cerr << "Error: Could not allocate memory for full P matrix" << std::endl;
        }

        for (size_t i = 0; i < numRows; i++) {
            for (int j = 0; j < numNeighbours; j++) {
                // std::cout << "(" << i << ", " << P.sample(i, j).index << "): " << P.sample(i, j).value << std::endl;
                full_P[i * numRows + P(i, j).index] = P(i, j).value;
            }
        }

        std::string filename = "P_matrix.txt";
        std::ofstream outFile(filename);

        if (!outFile) {
            std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        }

        outFile << std::fixed << std::setprecision(3);
        // Iterate over each row
        for (int i = 0; i < 100; ++i) {
            // Iterate over each column in the current row
            for (int j = 0; j < 100; ++j) {
                outFile << full_P[i * numRows + j];
                // Add a space between elements, but not after the last element
                if (j < 100 - 1) {
                    outFile << " ";
                }
            }
            // Newline after each row
            outFile << "\n";
        }

        // Close the file
        outFile.close();
        std::cout << "Array successfully written to " << filename << std::endl;

        free(full_P);
    }

    /*
    for (size_t i = 0; i < numRows; i++) {
        std::cout << "\nColumn: " << std::endl;
        for (unsigned int j = 0; j < numNeighbours; j++) {
            std::cout << P.sample(i, j).index << ": " << P.sample(i, j).value << ", ";
        }
    }*/

    // Normalizing the P matrix by dividing by the sum of all values in it

    std::cout << "Normalizing P" << std::endl;
    float P_sum = 0.0f;
    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < numNeighbours; j++) {
            P_sum += P(i, j).value;
        }
    }

    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < numNeighbours; j++) {
            P(i, j) = ValueAndIndex(P(i, j).value / P_sum, P(i, j).index);
        }
    }

    // Save the P matrix to cache
    std::ofstream cacheFileOut(cachePath, std::ios::binary | std::ios::out);
    if (cacheFileOut.is_open()) {
        // Save the P matrix to the cache file
        std::cout << "Saving P matrix to cache: " << cachePath << std::endl;

        cacheFileOut.write(reinterpret_cast<const char*>(P.begin()), P.shape[0] * P.shape[1] * sizeof(ValueAndIndex));

        for (int i = 0; i < numRows * numNeighbours; i++) {
            //cacheFileOut << P.data[i].value << " " << P.data[i].index << "\n";
        }
    }

    return P;
}

float PMatrixCalculator::squareDistance(float* a, float* b)
{
    float sqDist = 0.0f;
    for (int i = 0; i < numCols; i++)
    {
        float d = a[i] - b[i];
        sqDist += d * d;
    }
    return sqDist;
}

int PMatrixCalculator::getNumNeighbours(int num_samples, float perplexity)
{
    return glm::min(num_samples - 1, (int)(3 * perplexity) + 1);
}

NDArray<ValueAndIndex> PMatrixCalculator::getSquareDistances(float* data, int numNeighbours)
{
    /*
     * There are several implementations for calculating the distances to the `numNeighbours` nearest neighbours.
     * The common idea is to calculate the distances between each pair of points and then find the n closest ones for each point.
     * 
     * The sorting based implementation sorts the distances for each point and then selects the n closest ones.
     * This is quite efficient for a large number of neighbours, but sorting the entire array is expensive.
     * So for a small number of neighbours or very many points, the sorting based implementation is not optimal.
     * 
     * The naive implementation calculates the distances for each pair of points and then selects the n closest ones.
     * It does this by iterating over the distances and selecting the smallest one n times.
     * After each iteration, the smallest distance is set to infinity to avoid selecting it again.
     * This is quite inefficient for a large number of neighbours, but it's linear in the number of points.
     * 
     * The GPU based implementation performs the same calculations as the naive implementation, but on the GPU.
     * This is done by placing all data into SSBOs and then running compute shaders to calculate the distances.
     * This is quite fast but fairly complex due to the restrictions of runtime of a single shader pass.
     * The maximum runtime is about 2 seconds meaning not all data can be processed in a single pass.
     * The complexity arises from the need to split the calculation into multiple passes.
     * Also, the maximum size of the SSBOs is limited to 128 MB, so the data must be split into multiple SSBOs.
     * As a high level overview, the calculation is split as follows:
     * For some number of points at a time in a batch, calculate the distances to all other points (also batched).
     * Then do the naive selection in another pass.
    */

    // These variables can be used to choose the implementation to use
    // Notably, the GPU based implementation requires OpenGL 4.3 or higher and cannot run on mac
    // due to the lack of support of SSBOs.
    bool useGPU { true };
    bool useSorting { false };
    bool useNaive { false };
    
    // Perform verification of the different implementations by comparing them 
    // (specifically the naive and the GPU based one)
    // `useGPU` and `useNaive` must be true for this to work
    bool performVerification { false };

    // 2D arrays to store the results of the different implementations
    NDArray<ValueAndIndex> sqDistancesGPU { numRows, numNeighbours, useGPU };
    NDArray<ValueAndIndex> sqDistancesSorting { numRows, numNeighbours, useSorting };
    NDArray<ValueAndIndex> sqDistancesNaive { numRows, numNeighbours, useNaive };

    if (useGPU) {
        float startTime = static_cast<float>(getTime());

        // If the code crashes due to taking too long per iteration, decrease these values
        // However, if the code is slow, increasing these values may speed it up
        // Find the balance between speed and stability!
        unsigned int rowsPerIteration { 1000 };
        unsigned int rowsPerIterationInner { 5000 };
        GLint size;
        glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &size);

        // Writing our data into an SSBO
        unsigned int dataSSBO { 0 };
        glGenBuffers(1, &dataSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, dataSSBO);
        std::cout << "Allocating " << (numRows * numCols * sizeof(float)) << " bytes for data"
                  << " (usage " << (float)(numRows * numCols * sizeof(float)) / size * 100 << "%)"
            << std::endl;
        glBufferData(GL_SHADER_STORAGE_BUFFER, numRows * numCols * sizeof(float), data, GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, dataSSBO);

        // Making an SSBO for the square distances
        unsigned int sqDistancesSSBO { 0 };
        glGenBuffers(1, &sqDistancesSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, sqDistancesSSBO);
        std::cout << "Allocating " << (numRows * numNeighbours * sizeof(ValueAndIndex)) << " bytes for square distances" 
            << " (usage " << (float)(numRows * numNeighbours * sizeof(ValueAndIndex)) / size * 100 << "%)"
            << std::endl;
        glBufferData(GL_SHADER_STORAGE_BUFFER, numRows * numNeighbours * sizeof(ValueAndIndex), nullptr, GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sqDistancesSSBO);

        // Making an SSBO for all distances
        unsigned int allDistancesSSBO { 0 };
        glGenBuffers(1, &allDistancesSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, allDistancesSSBO);
        std::cout << "Allocating " << (rowsPerIteration * numRows * sizeof(ValueAndIndex)) << " bytes for all distances"
                  << " (usage " << (float)(rowsPerIteration * numRows * sizeof(ValueAndIndex)) / size * 100 << "%)"
                << std::endl;
        glBufferData(GL_SHADER_STORAGE_BUFFER, rowsPerIteration * numRows * sizeof(ValueAndIndex), nullptr, GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, allDistancesSSBO);

        // OpenGL error check loop
        GLenum error = glGetError();
        while (error != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << error << std::endl;
            error = glGetError();
        }

        // The shaders for the different parts of the calculation
        ComputeShader allDistancesShader(RESOURCE_ROOT "shaders_source/allDistancesCompute.shader");
        ComputeShader squareDistancesShader(RESOURCE_ROOT "shaders_source/squareDistancesCompute.shader");

        // Setting important parameters for the shaders
        squareDistancesShader.use();
        squareDistancesShader.setInt("numRows", numRows);
        squareDistancesShader.setInt("numCols", numCols);
        squareDistancesShader.setInt("numNeighbours", numNeighbours);

        allDistancesShader.use();
        allDistancesShader.setInt("numRows", numRows);
        allDistancesShader.setInt("numCols", numCols);
        allDistancesShader.setInt("numNeighbours", numNeighbours);
        allDistancesShader.setInt("rowsPerIteration", rowsPerIteration);

        // The number of iterations needed to calculate all distances
        // This calculation is split into chunks of `rowsPerIteration` rows
        unsigned int allDistancesIterations = (numRows + rowsPerIteration - 1) / rowsPerIteration;
        float allDistancesStartTime = getTime();

        for (unsigned int i = 0; i < allDistancesIterations; i++)
        {
            allDistancesShader.use();

            unsigned int rowsIterationOffset = i * rowsPerIteration;
            allDistancesShader.setInt("rowsIterationOffset", rowsIterationOffset);
            allDistancesShader.setInt("rowsPerIterationInner", rowsPerIterationInner);

            // The number of iterations required for this inner loop
            // This is needed to split the calculation into chunks of `rowsPerIterationInner` rows
            // This is done to avoid the maximum runtime of 2 seconds for a single shader pass
            unsigned int iterationsInner = (numRows + rowsPerIterationInner - 1) / rowsPerIterationInner;
            for (unsigned int j = 0; j < iterationsInner; j++)
            {
                unsigned int rowsIterationOffsetInner = j * rowsPerIterationInner;
                allDistancesShader.setInt("rowsIterationOffsetInner", rowsIterationOffsetInner);
                allDistancesShader.run((rowsPerIteration + 7) / 8);

                std::cout << (j + 1) << " / " << iterationsInner << std::endl;

                // Swap the buffers in order to avoid the shader time limit
                // By swapping the buffers, we show that the device has not hung
                glfwSwapBuffers(WindowHandler::getWindow());
            }

            // Running the calculation for the square distances based on the list of all distances
            squareDistancesShader.use();
            squareDistancesShader.setInt("rowsIterationOffset", rowsIterationOffset);
            squareDistancesShader.run((rowsPerIteration + 7) / 8);

            // This part is just for printing out some information about the progress
            float timeTaken = getTime() - allDistancesStartTime;
            float totalTimeEstimate = timeTaken / ((float)(i + 1) / (float)(allDistancesIterations));

            std::cout << "iteration: " 
                << (i + 1) << "/" << allDistancesIterations
                << " [" 
                << int(timeTaken) / 60 << "m" << (int(timeTaken) % 60) << "s "
                <<"/ " 
                << int(totalTimeEstimate) / 60 << "m" << (int(totalTimeEstimate) % 60) << "s"
                << "]"
                << ": [" << rowsIterationOffset << " - " << (rowsIterationOffset + rowsPerIteration) 
                << "] of " << numRows << std::endl;

            // Swap the buffers in order to avoid the shader time limit
            // By swapping the buffers, we show that the device has not hung
            glfwSwapBuffers(WindowHandler::getWindow());
        }

        // Reading the data back from the SSBO
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, sqDistancesSSBO);
        ValueAndIndex* data = (ValueAndIndex*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

        for (int i = 0; i < numRows; i++) {
            for (int j = 0; j < numNeighbours; j++) {
                sqDistancesGPU(i, j) = data[i * numNeighbours + j];
            }
        }

        // Freeing the memory of the SSBOs
        glDeleteBuffers(1, &dataSSBO);
        glDeleteBuffers(1, &sqDistancesSSBO);
        glDeleteBuffers(1, &allDistancesSSBO);

        float endTime = getTime();
        std::cout << "Time taken (GPU): " << endTime - startTime << " seconds" << std::endl;
    }

    // n points
    // m neighbours
    // d dimensions
    // O (n * (n * d + n * log n + m))
    if (useSorting) {

        float startTime = getTime();
        startTime = getTime();
#pragma omp parallel for
        for (int i = 0; i < numRows; i++) {
            std::vector<ValueAndIndex> allDistances(numRows);

            // Calculate all distances to this point
            for (int j = 0; j < numRows; j++) {
                if (i == j) {
                    allDistances[j] = ValueAndIndex(std::numeric_limits<float>::infinity(), j); // Ignore distance to itself
                    continue;
                }

                float sqDistance = 0.0f;

                // Calculate squared Euclidean distance
                for (int k = 0; k < numCols; k++) {
                    float diff = data[i * numCols + k] - data[j * numCols + k];
                    sqDistance += diff * diff;
                }

                allDistances[j] = ValueAndIndex(sqDistance, j);
            }

            // Sorting all distances to this point
            std::sort(allDistances.begin(), allDistances.end(), [](const ValueAndIndex& a, const ValueAndIndex& b) {
                return a.value < b.value;
            });

            // Select the n closest neighbours
            for (int j = 0; j < numNeighbours; j++) {
                sqDistancesSorting(i, j) = allDistances[j];
            }
        }

        float endTime = getTime();
        std::cout << "Time taken (sorting): " << endTime - startTime << " seconds" << std::endl;
    }

    // n points
    // m neighbours
    // d dimensions
    // O (n * (n * d + n * m))
    if (useNaive) {
        float startTime = getTime();

        #pragma omp parallel for
        for (int i = 0; i < numRows; i++) {
            NDArray<ValueAndIndex> allDistances = NDArray<ValueAndIndex>::empty({ numRows });

            // Calculate all distances to this point
            for (int j = 0; j < numRows; j++) {
                // Ignore distance to itself
                if (i == j) {
                    allDistances(j) = ValueAndIndex(std::numeric_limits<float>::infinity(), j); // Ignore distance to itself
                    continue;
                }

                float sqDistance = 0.0f;

                // Calculate squared Euclidean distance
                for (int k = 0; k < numCols; k++) {
                    float diff = data[i * numCols + k] - data[j * numCols + k];
                    sqDistance += diff * diff;
                }

                allDistances(j) = ValueAndIndex(sqDistance, j);
            }

            // Iteratively selecting the n closest neighbours
            for (int j = 0; j < numNeighbours; j++) {
                ValueAndIndex lowestDistance = allDistances(0);
                int lowestIndex = 0;

                // Finding the current lowest distance
                for (int k = 1; k < numRows; k++) {
                    if (allDistances(k).value <= lowestDistance.value) {
                        lowestDistance = allDistances(k);
                        lowestIndex = k;
                    }
                }

                // Selecting the lowest distance
                sqDistancesNaive(i, j) =  lowestDistance;

                // Setting the lowest distance to infinity to avoid selecting it again
                allDistances(lowestIndex).value = std::numeric_limits<float>::infinity();
            }
        }

        float endTime = getTime();
        std::cout << "Time taken (naive): " << endTime - startTime << " seconds" << std::endl;
    }

    if (performVerification)
    {
        // Verifying that the naive and GPU based implementations are the same
        // by comparing each of their elements
        // They may differ very slightly due to the differences in CPU and GPU floating point precision
        // Therefore we often see elements with the same values being swapped
        // These errors can be ignored as neither configuration is wrong
        for (int i = 0; i < numRows; i++)
        {
            for (int j = 0; j < numNeighbours; j++)
            {
                float v1 = sqDistancesGPU(i, j).value;
                float v2 = sqDistancesNaive(i, j).value;

                if (sqDistancesGPU(i, j).index != sqDistancesNaive(i, j).index || glm::abs(v1 - v2) > 0.01f) {
                    std::cerr 
                        << "Mismatch at (" << i << ", " << j << "): " 
                        << sqDistancesGPU(i, j).index << " != " << sqDistancesNaive(i, j).index
                        << " or "
                        << sqDistancesGPU(i, j).value << " != " << sqDistancesNaive(i, j).value 
                        << std::endl;
                }
            }
        }
    }

    /*
    for (size_t i = numRows - 100; i < numRows; i++) {
        std::cout << "Column: " << std::endl;
        for (unsigned int j = 0; j < numNeighbours; j++) {
            std::cout << sqDistancesGPU.sample(i, j).index << ": " << sqDistancesGPU.sample(i, j).value << ", ";
        }
    }*/
    

    if (useNaive)
    {
        return sqDistancesNaive;
    }
    if (useSorting)
    {
        return sqDistancesSorting;
    }
    return sqDistancesGPU;
}

NDArray<ValueAndIndex> PMatrixCalculator::binarySearchPerplexity(NDArray<ValueAndIndex> sqDistances, float perplexity)
{
    /*
     * Copied from the original t-SNE implementation (translated to c++)
    */

    unsigned int num_steps = 100;
    unsigned int num_samples = sqDistances.shape[0];
    unsigned int num_neighbours = sqDistances.shape[1];
    float beta = 0.0f;
    float beta_min = 0.0f;
    float beta_max = 0.0f;
    float beta_sum = 0.0f;

    float desired_entropy = glm::log(perplexity);
    float entropy_diff = 0.0f;

    float entropy = 0.0f;
    float sum_Pi = 0.0f;
    float sum_disti_Pi = 0.0f;

    NDArray<ValueAndIndex> P = NDArray<ValueAndIndex>::empty({ num_samples, num_neighbours });

    constexpr float fmin = std::numeric_limits<float>::min();
    constexpr float fmax = std::numeric_limits<float>::max();

    for (unsigned int i = 0; i < num_samples; i++)
    {
        beta_min = fmin;
        beta_max = fmax;
        beta = 1.0f;

        for (unsigned int step = 0; step < num_steps; step++)
        {
            sum_Pi = 0.0f;
            for (unsigned int j = 0; j < num_neighbours; j++)
            {
                if (sqDistances(i, j).index == i)
                    continue;

                float value = glm::exp(-sqDistances(i, j).value * beta);

                P(i, j) = ValueAndIndex(value, sqDistances(i, j).index);
                sum_Pi += value;
            }

            if (sum_Pi == 0.0f)
            {
                sum_Pi = 1e-8f;
            }
            sum_disti_Pi = 0.0f;

            for (unsigned int j = 0; j < num_neighbours; j++) {
                ValueAndIndex P_val = P(i, j);
                P_val.value /= sum_Pi;
                P(i, j) = P_val;
                sum_disti_Pi += sqDistances(i, j).value * P_val.value;
            }

            entropy = glm::log(sum_Pi) + beta * sum_disti_Pi;
            entropy_diff = entropy - desired_entropy;

            if (glm::abs(entropy_diff) <= 1e-5)
            {
                break;
            }

            
            if (entropy_diff > 0.0f)
            {
                beta_min = beta;
                if (beta_max == fmax) {
                    beta *= 2.0f;
                }
                else {
                    beta = (beta + beta_max) / 2.0f;
                }
            } else {
                beta_max = beta;
                if (beta_min == fmin) {
                    beta /= 2.0f;
                }
                else {
                    beta = (beta + beta_min) / 2.0f;
                }
            }
        }

        beta_sum += beta;
    }
    
    /*
    for (size_t i = 0; i < numRows; i++) {
        std::cout << "\nColumn: " << std::endl;
        for (unsigned int j = 0; j < num_neighbours; j++) {
            std::cout << P.sample(i, j).index << ": " << P.sample(i, j).value << ", ";
        }
    }*/
    

    return P;
}

float PMatrixCalculator::getTime()
{
    return static_cast<float>(glfwGetTime());
}
