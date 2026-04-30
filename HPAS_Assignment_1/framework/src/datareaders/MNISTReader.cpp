#include "datareaders/MNISTReader.h"


MNISTReader::MNISTReader()
{

}

MNISTReader::~MNISTReader()
{

}

void MNISTReader::readData(TSNE& tsne, SquareDistanceCalculator& squareDistanceCalculator, Settings& settings, int numNeighbours, bool bigSet)
{
    int numImages, numRows, numCols;
	const char* imageFile = bigSet 
        ? RESOURCE_ROOT "../resources/datasets/mnist/train-images.idx3-ubyte" 
        : RESOURCE_ROOT "../resources/datasets/mnist/t10k-images.idx3-ubyte";
    float* images = readImages(imageFile, numImages, numRows, numCols);

    int numLabels;
	const char* labelFile = bigSet
		? RESOURCE_ROOT "../resources/datasets/mnist/train-labels.idx1-ubyte"
		: RESOURCE_ROOT "../resources/datasets/mnist/t10k-labels.idx1-ubyte";
    std::vector<int> labels = readLabels(labelFile, numLabels);

    std::cout << "Successfully read " << numImages << " images of size "
        << numRows << "x" << numCols << " and " << numLabels << " labels." << std::endl;

	NDArray<float> data = NDArray<float>::empty({ (size_t)numImages, (size_t)numRows * numCols });

    for (int i = 0; i < numImages; i++)
    {
        for (int j = 0; j < numRows * numCols; ++j)
        {
			data(i, j) = images[i * numRows * numCols + j];
        }
    }

	NearestNeighbourCalculator::calculateNearestNeighbours("MNIST10k", data, 30);

    unsigned int numPoints = numImages;

    PMatrixCalculator pMatrixCalculator;
    PMatrix P = pMatrixCalculator.getPMatrix(images, numPoints, numRows * numCols, (float)numNeighbours, squareDistanceCalculator, ("MNIST" + std::to_string(bigSet)).c_str());

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
    tsne.setLabels(labels);

    // Free the allocated memory
    free(images);
}

// Function to read the 4-byte integers from the file in MSB-first format
int MNISTReader::readInt(std::ifstream& file)
{
    int value;
    file.read(reinterpret_cast<char*>(&value), sizeof(value));
    return ntohl(value); // Convert from network to host byte order (big-endian to little-endian if needed)
}

// Function to read MNIST image data into a float array
float* MNISTReader::readImages(const std::string& filename, int& numImages, int& numRows, int& numCols)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Error opening file: " << filename << std::endl;
        return nullptr;
    }

    int magicNumber = readInt(file);
    if (magicNumber != 2051)
    { // Check if magic number matches the image file format
        std::cerr << "Invalid magic number for image file: " << magicNumber << std::endl;
        return nullptr;
    }

    numImages = readInt(file);
    numRows = readInt(file);
    numCols = readInt(file);

    int imageSize = numRows * numCols;
    float* images = static_cast<float*>(malloc(numImages * imageSize * sizeof(float)));
    if (!images)
    {
        std::cerr << "Failed to allocate memory for images" << std::endl;
        return nullptr;
    }

    unsigned char pixel;
    for (int i = 0; i < numImages * imageSize; ++i)
    {
        file.read(reinterpret_cast<char*>(&pixel), 1);
        images[i] = static_cast<float>(pixel) / 255.0f; // Normalize to [0, 1]
    }

    file.close();
    return images;
}

// Function to read MNIST label data into an integer array
std::vector<int> MNISTReader::readLabels(const std::string& filename, int& numLabels)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Error opening file: " << filename << std::endl;
        return std::vector<int>();
    }

    int magicNumber = readInt(file);
    if (magicNumber != 2049)
    {
        // Check if magic number matches the label file format
        std::cerr << "Invalid magic number for label file: " << magicNumber << std::endl;
        return std::vector<int>();
    }

    numLabels = readInt(file);

	std::vector<int> labels(numLabels);

    unsigned char label;
    for (int i = 0; i < numLabels; ++i)
    {
        file.read(reinterpret_cast<char*>(&label), 1);
        labels[i] = label;
    }

    file.close();
    return labels;
}
