#include "GPUSquareDistanceCalculator.h"

GPUSquareDistanceCalculator::GPUSquareDistanceCalculator()
{}

GPUSquareDistanceCalculator::~GPUSquareDistanceCalculator()
{}

NDArray<ValueAndIndex> GPUSquareDistanceCalculator::getSquareDistances(float* data, int numRows, int numCols, int numNeighbours)
{
    NDArray<ValueAndIndex> sqDistances = NDArray<ValueAndIndex>::empty({ numRows, numNeighbours });
    float startTime = static_cast<float>(getTime());

    // If the code crashes due to taking too long per iteration, decrease these values
    // However, if the code is slow, increasing these values may speed it up
    // Find the balance between speed and stability!
    unsigned int rowsPerIteration{ 1000 };
    unsigned int rowsPerIterationInner{ 5000 };
    GLint size;
    glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &size);

    // Writing our data into an SSBO
    unsigned int dataSSBO{ 0 };
    glGenBuffers(1, &dataSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, dataSSBO);
    std::cout << "Allocating " << (numRows * numCols * sizeof(float)) << " bytes for data"
        << " (usage " << (float)(numRows * numCols * sizeof(float)) / size * 100 << "%)"
        << std::endl;
    glBufferData(GL_SHADER_STORAGE_BUFFER, numRows * numCols * sizeof(float), data, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, dataSSBO);

    // Making an SSBO for the square distances
    unsigned int sqDistancesSSBO{ 0 };
    glGenBuffers(1, &sqDistancesSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sqDistancesSSBO);
    std::cout << "Allocating " << (numRows * numNeighbours * sizeof(ValueAndIndex)) << " bytes for square distances"
        << " (usage " << (float)(numRows * numNeighbours * sizeof(ValueAndIndex)) / size * 100 << "%)"
        << std::endl;
    glBufferData(GL_SHADER_STORAGE_BUFFER, numRows * numNeighbours * sizeof(ValueAndIndex), nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sqDistancesSSBO);

    // Making an SSBO for all distances
    unsigned int allDistancesSSBO{ 0 };
    glGenBuffers(1, &allDistancesSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, allDistancesSSBO);
    std::cout << "Allocating " << (rowsPerIteration * numRows * sizeof(ValueAndIndex)) << " bytes for all distances"
        << " (usage " << (float)(rowsPerIteration * numRows * sizeof(ValueAndIndex)) / size * 100 << "%)"
        << std::endl;
    glBufferData(GL_SHADER_STORAGE_BUFFER, rowsPerIteration * numRows * sizeof(ValueAndIndex), nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, allDistancesSSBO);

    // OpenGL error check loop
    GLenum error = glGetError();
    while (error != GL_NO_ERROR)
    {
        std::cerr << "OpenGL error: " << error << std::endl;
        error = glGetError();
    }

    // The shaders for the different parts of the calculation
    ComputeShader allDistancesShader(RESOURCE_ROOT "framework/shaders_source/allDistancesCompute.shader");
    ComputeShader squareDistancesShader(RESOURCE_ROOT "framework/shaders_source/squareDistancesCompute.shader");

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
            << "/ "
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

    ValueAndIndex* returned_data = (ValueAndIndex*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

	for (int i = 0; i < numRows; i++)
	{
		for (int j = 0; j < numNeighbours; j++)
		{
			sqDistances(i, j) = returned_data[i * numNeighbours + j];
		}
	}

    // Freeing the memory of the SSBOs
    glDeleteBuffers(1, &dataSSBO);
    glDeleteBuffers(1, &sqDistancesSSBO);
    glDeleteBuffers(1, &allDistancesSSBO);

    float endTime = getTime();
    std::cout << "Time taken (GPU): " << endTime - startTime << " seconds" << std::endl;

    return sqDistances;
}
