#include "PCA.h"

NDArray<float> computeMeans(NDArray<float>& data)
{
	NDArray<float> means = NDArray<float>::empty({ data.shape[0] });

#pragma omp parallel for
	for (int i = 0; i < data.shape[0]; ++i)
	{
		// Compute the mean for each row by summing the elements and dividing by the number of elements
		float mean = 0.0f;
		for (size_t j = 0; j < data.shape[1]; ++j)
		{
			mean += data(i, j);
		}
		mean /= data.shape[1];
		means(i) = mean;
	}

	return means;
}

NDArray<float> computeStandardDeviations(NDArray<float> means, NDArray<float>& data)
{
	NDArray<float> stddevs = NDArray<float>::empty({ data.shape[0] });

#pragma omp parallel for
	for (int i = 0; i < data.shape[0]; ++i)
	{
		// The variance is the average of the squared differences from the Mean
		float variance = 0.0f;
		for (size_t j = 0; j < data.shape[1]; ++j)
		{
			float deviationFromMean = data(i, j) - means(i);
			variance += deviationFromMean * deviationFromMean;
		}
		variance /= data.shape[0];

		if (variance == 0.0f)
		{
			variance = 1.0f;
		}

		// The standard deviation is the square root of the variance
		float stddev = std::sqrt(variance);
		stddevs(i) = stddev;
	}

	return stddevs;
}

NDArray<float> normalizeData(NDArray<float>& data, NDArray<float>& means, NDArray<float>& standardDeviations)
{
	NDArray<float> normalizedData = NDArray<float>::empty(data.shape);

	// For each data point,
#pragma omp parallel for
	for (int i = 0; i < data.shape[0]; ++i)
	{
		for (size_t j = 0; j < data.shape[1]; ++j)
		{
			// Subtract mean and divide by standard deviation to normalize
			normalizedData(i, j) = (data(i, j) - means(i)) / standardDeviations(i);
		}
	}

	return normalizedData;
}

NDArray<float> computeCovarianceMatrix(NDArray<float>& data, NDArray<float>& means)
{
	NDArray<float> covarianceMatrix = NDArray<float>::empty({ data.shape[1], data.shape[1] });

	int numSamples = data.shape[0];

	int stride = 1;

	if (data.shape[1] > 500)
	{
		stride = 20;
	}

#pragma omp parallel for
	for (int i = 0; i < data.shape[1]; ++i)
	{
		for (int j = 0; j < data.shape[1]; ++j)
		{
			float covariance = 0.0f;
			// Loop through each data point
			for (size_t k = 0; k < numSamples; k += stride)
			{
				// Calculate covariance using the formula
				covariance += (data(k, i) - means(i)) * (data(k, j) - means(j));
			}
			covariance /= ((numSamples / stride) - 1);
			covarianceMatrix(i, j) = covariance;
		}
	}

	return covarianceMatrix;
}

NDArray<float> getRandomVectorOfLength(unsigned int length)
{
	NDArray<float> randomVector = NDArray<float>::empty({ length });
	for (unsigned int i = 0; i < length; ++i)
	{
		randomVector(i) = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	}
	return randomVector;
}

NDArray<float> multiply(NDArray<float>& a, NDArray<float>& b)
{
	// matrix/vector case:
	bool isMatrixVectorCase = a.shape.size() == 2 && b.shape.size() == 1;
	if (isMatrixVectorCase)
	{
		b.reshape(b.shape[0], 1);
	}

	// Check that the dimensions are compatible for matrix multiplication
	if (a.shape[1] != b.shape[0])
	{
		throw std::invalid_argument("Incompatible dimensions for matrix multiplication");
	}

	NDArray<float> result = NDArray<float>::empty({ a.shape[0], b.shape[1] });

#pragma omp parallel for
	for (int i = 0; i < a.shape[0]; ++i)
	{
		for (size_t j = 0; j < b.shape[1]; ++j)
		{
			float sum = 0.0f;
			for (size_t k = 0; k < a.shape[1]; ++k)
			{
				sum += a(i, k) * b(k, j);
			}
			result(i, j) = sum;
		}
	}

	if (isMatrixVectorCase)
	{
		b.reshape(b.shape[0]);
	}

	return result;
}

NDArray<float> transpose(NDArray<float>& v)
{
	// Vector case
	if (v.shape.size() == 1)
	{
		NDArray<float> transposed = v;
		transposed.reshape((size_t)1, v.shape[0]);

		return transposed;
	}

	// Matrix case
	if (v.shape.size() == 2)
	{
		NDArray<float> transposed = NDArray<float>::empty({ v.shape[1], v.shape[0] });

#pragma omp parallel for
		for (int i = 0; i < v.shape[0]; ++i)
		{
			for (size_t j = 0; j < v.shape[1]; ++j)
			{
				transposed(j, i) = v(i, j);
			}
		}

		return transposed;
	}

	throw std::invalid_argument("Invalid shape for transpose");
}

NDArray<float> divide(NDArray<float>& a, float b)
{
	// Vector case
	if (a.shape.size() == 1)
	{
		NDArray<float> result = NDArray<float>::empty(a.shape);

#pragma omp parallel for
		for (int i = 0; i < a.shape[0]; ++i)
		{
			result(i) = a(i) / b;
		}

		return result;
	}

	// Matrix case
	if (a.shape.size() == 2)
	{
		NDArray<float> result = NDArray<float>::empty(a.shape);

#pragma omp parallel for
		for (int i = 0; i < a.shape[0]; ++i)
		{
			for (size_t j = 0; j < a.shape[1]; ++j)
			{
				result(i, j) = a(i, j) / b;
			}
		}

		return result;
	}

	throw std::invalid_argument("Invalid shape for division");
}

NDArray<float> subtract(NDArray<float>& a, NDArray<float>& b)
{
	// Vector case
	if (a.shape.size() == 1 && b.shape.size() == 1)
	{
		if (a.shape[0] != b.shape[0])
		{
			throw std::invalid_argument("Incompatible dimensions for subtraction");
		}

		NDArray<float> result = NDArray<float>::empty(a.shape);

#pragma omp parallel for
		for (int i = 0; i < a.shape[0]; ++i)
		{
			result(i) = a(i) - b(i);
		}

		return result;
	}

	// Matrix case
	if (a.shape.size() == 2 && b.shape.size() == 2)
	{
		if (a.shape[0] != b.shape[0] || a.shape[1] != b.shape[1])
		{
			throw std::invalid_argument("Incompatible dimensions for subtraction");
		}

		NDArray<float> result = NDArray<float>::empty(a.shape);

#pragma omp parallel for
		for (int i = 0; i < a.shape[0]; ++i)
		{
			for (size_t j = 0; j < a.shape[1]; ++j)
			{
				result(i, j) = a(i, j) - b(i, j);
			}
		}

		return result;
	}

	throw std::invalid_argument("Invalid shape for subtraction");
}

float computeNorm(NDArray<float>& v)
{
	float norm = 0.0f;
	for (int i = 0; i < v.shape[0]; ++i)
	{
		norm += v(i) * v(i);
	}
	return std::sqrt(norm);
}

void computeEigenvaluesAndEigenvectors(NDArray<float>& covarianceMatrix, NDArray<float>& eigenvalues, NDArray<float>& eigenvectors, size_t numComponents)
{
	for (int componentIndex = 0; componentIndex < numComponents; ++componentIndex)
	{
		NDArray<float> v = getRandomVectorOfLength(covarianceMatrix.shape[0]);

		for (unsigned int i = 0; i < 100; ++i)
		{
			// Compute the matrix-vector product
			NDArray<float> vNew = multiply(covarianceMatrix, v);

			vNew.reshape(vNew.shape[0]);

			// Normalize the vector
			float norm = computeNorm(vNew);

			vNew = divide(vNew, norm);
			
			v = vNew;
		}

		// Compute the eigenvalue
		NDArray<float> vT = transpose(v);
		NDArray<float> covTimesV = multiply(covarianceMatrix, v);
		NDArray<float> eigenvalue = multiply(vT, covTimesV);
		eigenvalues(componentIndex) = eigenvalue(0, 0); // Holds the eigenvalue approximation

		for (unsigned int i = 0; i < v.shape[0]; ++i)
		{
			eigenvectors(componentIndex, i) = v(i);
		}

		// Deflation
		NDArray<float> v_reshaped_as_matrix = v;
		v_reshaped_as_matrix.reshape(v.shape[0], 1);
		NDArray<float> v_reshaped_as_matrixT = transpose(v_reshaped_as_matrix);
		NDArray<float> outerProduct = multiply(v_reshaped_as_matrix, v_reshaped_as_matrixT);
		NDArray<float> covTimesOuterProduct = multiply(covarianceMatrix, outerProduct);
		NDArray<float> covarianceMatrixDeflated = subtract(covarianceMatrix, covTimesOuterProduct);
		covarianceMatrix = covarianceMatrixDeflated;
	}
}

std::vector<glm::dvec2> PCA::embed(NDArray<float>& data)
{
	srand(0);

	std::cout << "Performing PCA:" << std::endl;

	// 1. Standandardize the data
	START_TIMER();
	NDArray<float> means = computeMeans(data);
	NDArray<float> standardDeviations = computeStandardDeviations(means, data);
	NDArray<float> normalizedData = normalizeData(data, means, standardDeviations);
	LOG_TIME("Standardization");

	// 2. Compute the covariance matrix
	//NDArray<float> covarianceMatrix = computeCovarianceMatrix(normalizedData, means);
	//NDArray<float> transposedNormalizedData = transpose(normalizedData); // shape: (features, samples)
	//NDArray<float> covarianceMatrix = multiply(transposedNormalizedData, normalizedData); // (features � features)
	//covarianceMatrix = divide(covarianceMatrix, (float)(normalizedData.shape[0] - 1));
	NDArray<float> covarianceMatrix = computeCovarianceMatrix(normalizedData, means);

	LOG_TIME("Compute covariance matrix");

	// 3. Compute the eigenvalues and eigenvectors
	size_t numComponents = 2;
	NDArray<float> eigenvectors = NDArray<float>::empty({ numComponents, data.shape[1] });
	NDArray<float> eigenvalues = NDArray<float>::empty({ numComponents });
	computeEigenvaluesAndEigenvectors(covarianceMatrix, eigenvalues, eigenvectors, numComponents);
	LOG_TIME("Compute eigenvectors");

	// 4. Project the data onto the eigenvectors
	NDArray<float> W = transpose(eigenvectors);
	NDArray<float> projectedData = multiply(normalizedData, W);
	LOG_TIME("Project data");

	// 5. Return the projected data
	std::vector<glm::dvec2> result(projectedData.shape[0]);
	for (int i = 0; i < projectedData.shape[0]; ++i)
	{
		glm::dvec2 point;
		point.x = projectedData(i, 0);
		point.y = projectedData(i, 1);
		result[i] = point;
	}

	return result;
}
