#pragma once

#include <vector>
#include <array>
#include <fstream>

#define IS_FORMAT_SUPPORTED (__GNUC__ >= 13 || __clang_major__ >= 17 || _MSC_VER >= 1929 || (__APPLE__ && __clang_major__ >= 15))

#if IS_FORMAT_SUPPORTED
#include <format>
/*
 * For Clang 14 and above, there is incomplete support for std::format,
 * but for that, the experimental features of libc++ need to be enabled.
 */
#endif

#include <complex>
#include <glm/glm.hpp>

/**
 * A concept used for the constructor of NDArray
 */
template <typename T>
concept SizeType = std::is_same_v<int, T> || std::is_same_v<unsigned long, T> || std::is_same_v<unsigned int, T> || std::is_same_v<size_t, T>;

/**
 * A multi-dimensional array
 * @tparam T the datatype of the elements in the array
 */
template <typename T>
class NDArray {
private:
    std::vector<T> data;
    std::vector<size_t> strides;
    size_t dims;
public:
    NDArray();

    NDArray(std::initializer_list<T> init);
    NDArray(std::vector<T> init);
    ~NDArray() = default;
	// Move constructor to avoid copying the data
    NDArray(NDArray&& other) noexcept
        : data(std::move(other.data)), shape(std::move(other.shape)),
        strides(std::move(other.strides)), dims(other.dims)
    {}
    NDArray<T>& operator=(NDArray<T>&& other) noexcept
    {
        if (this != &other)
        {
            data = std::move(other.data);
            shape = std::move(other.shape);
            strides = std::move(other.strides);
            dims = other.dims;
        }
        return *this;
    }
    // Copy constructor
    NDArray(const NDArray<T>& other)
        : data(other.data), shape(other.shape), strides(other.strides), dims(other.dims)
    {}
    NDArray<T>& operator=(const NDArray<T>&other)
    {
        if (this != &other)
        {
            data = other.data;
            shape = other.shape;
            strides = other.strides;
            dims = other.dims;
        }
        return *this;
    }
    NDArray<T> clone() const
    {
        return NDArray<T>(*this); // Uses copy constructor
    }

    std::vector<size_t> shape;
    template <SizeType... Indices>
    T& operator()(Indices... indices);

    bool approxEquals(NDArray<T> other, double eps = 0.00005);

    template <SizeType ...S>
    void reshape(S... _dims);

    T* begin() { return data.data(); }

	size_t size() const { return data.size(); }

    static NDArray<T> empty(std::initializer_list<size_t> _dims) {
        size_t numElements = 1;

        NDArray<T> out = NDArray<T>();
        out.dims = _dims.size();

        out.shape = std::vector<size_t>(_dims);

        for (size_t i = 0; i < out.dims; i++)
            numElements *= out.shape[i];

        out.strides = std::vector<size_t>(out.dims);

        size_t curStride = 1;
        for (int i = static_cast<int>(out.dims) - 1; i >= 0; --i) {
            out.strides[i] = curStride;
            curStride *= out.shape[i];
        }
        out.data = std::vector<T>(numElements);

        return std::move(out);
    }

    static NDArray<T> empty(std::initializer_list<int> _dims)
    {
		std::vector<size_t> converted_dims;
        
		for (auto dim : _dims)
		{
			converted_dims.push_back(static_cast<size_t>(dim));
		}

		return empty(converted_dims);
    }

    static NDArray<T> empty(std::initializer_list<unsigned int> _dims)
    {
        std::vector<size_t> converted_dims;

        for (auto dim : _dims)
        {
            converted_dims.push_back(static_cast<size_t>(dim));
        }

        return empty(converted_dims);
    }

    static NDArray<T> empty(std::vector<size_t>& _dims)
    {
        size_t numElements = 1;

        NDArray<T> out = NDArray<T>();
        out.dims = _dims.size();

        out.shape = std::vector<size_t>(_dims);

        for (size_t i = 0; i < out.dims; i++)
            numElements *= out.shape[i];

        out.strides = std::vector<size_t>(out.dims);

        size_t curStride = 1;
        for (int i = static_cast<int>(out.dims) - 1; i >= 0; --i)
        {
            out.strides[i] = curStride;
            curStride *= out.shape[i];
        }
        out.data = std::vector<T>(numElements);

        return out;
    }

    void writeToFile(const std::string& filename) const
    {
        std::ofstream outFile(filename, std::ios::binary);
        if (!outFile)
        {
            throw std::runtime_error("Failed to open file for writing: " + filename);
        }

        // Write the number of dimensions
        outFile.write(reinterpret_cast<const char*>(&dims), sizeof(size_t));

        // Write the shape vector
        outFile.write(reinterpret_cast<const char*>(shape.data()), shape.size() * sizeof(size_t));

        // Write the data vector
        outFile.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(T));

        outFile.close();
    }

    static NDArray<T> readFromFile(const std::string& filename)
    {
        std::ifstream inFile(filename, std::ios::binary);
        if (!inFile)
        {
            throw std::runtime_error("Failed to open file for reading: " + filename);
        }

        size_t fileDims;
        inFile.read(reinterpret_cast<char*>(&fileDims), sizeof(size_t));

        std::vector<size_t> fileShape(fileDims);
        inFile.read(reinterpret_cast<char*>(fileShape.data()), fileDims * sizeof(size_t));

        size_t numElements = 1;
        for (size_t dim : fileShape)
        {
            numElements *= dim;
        }

        std::vector<T> fileData(numElements);
        inFile.read(reinterpret_cast<char*>(fileData.data()), numElements * sizeof(T));

        inFile.close();

        NDArray<T> result = NDArray<T>::empty(fileShape);
        result.data = std::move(fileData);

        return result;
    }

    static double diff(double a, double b)
    {
        return abs(a - b);
    }

    static double diff(float a, float b)
    {
        return abs(a - b);
    }

    static double diff(int a, int b)
    {
        return abs(a - b);
    }

    static double diff(glm::dvec2 a, glm::dvec2 b)
    {
        return glm::length(a - b);
    }

    static double diff(std::complex<double> a, std::complex<double> b)
    {
        return abs(a.real() - b.real()) + abs(a.imag() - b.imag());
    }
};

// Implementations of class NDArray's methods

template <typename T>
NDArray<T>::NDArray(){

    data = std::vector<T>();
    shape = std::vector<size_t>();
    strides = std::vector<size_t>();
    dims = 0;
}

/**
 * Initialize a 1-D NDArray using a list of elements provided in brackets notation {...}
 * @param init
 */
template <typename T>
NDArray<T>::NDArray(std::initializer_list<T> init) {

    data = std::vector<T>(init);
    shape = std::vector<size_t>(1);
    strides = std::vector<size_t>(1);
    dims = 1;
    shape[0] = init.size();
    strides[0] = 1;
}

/**
 * Initialize a 1-D NDArray using a predetermined std::vector of elements
 * @param elements
 */
template <typename T>
NDArray<T>::NDArray(std::vector<T> elements) {

    shape = std::vector<size_t>(1);
    strides = std::vector<size_t>(1);
    dims = 1;
    shape[0] = elements.size();
    strides[0] = 1;
    data = std::move(elements);
}


/**
 * Addressing the array
 * @tparam Indices check definition of SizeType -- either of those types
 * @param indices variable-length list of indices
 * @return
 */
template <typename T>
template <SizeType... Indices>
T& NDArray<T>::operator()(Indices... indices) {
    /*
     * Note that there is no implemented RW lock, so if multiple threads write
     * to the NDArray, there won't be any races as long as they all write to different
     * addresses. For the purposes of the assignment and for using OpenMP,
     * no concurrency issues are expected.
     */
    size_t calculatedIdx = 0;
    std::initializer_list<size_t> index_list = { static_cast<size_t>(indices)... };

	// Check if the number of indices matches the number of dimensions
    if (sizeof...(indices) != dims)
    {
#if IS_FORMAT_SUPPORTED
        throw std::invalid_argument(std::format("Incorrect number of indices. Expected {} but got {}", dims, sizeof...(indices)));
#else
        throw std::invalid_argument("Incorrect number of indices. Expected " + std::to_string(dims) + " but got " + std::to_string(sizeof...(indices)));
#endif
    }

    size_t calculatedIndex = 0;
    size_t i = 0;

	// Calculate the index in the 1D array
    for (size_t idx : index_list)
    {
		// Check if the index is within bounds. Since size_t is unsigned, we don't need to check for negative values
        if (idx >= shape[i])
        {
#if IS_FORMAT_SUPPORTED
            throw std::out_of_range(std::format("Index {} out of bounds for dimension {} (size {}).", idx, i, shape[i]));
#else
            throw std::out_of_range("Index " + std::to_string(idx) + " out of bounds for dimension " + std::to_string(i) + " (size " + std::to_string(shape[i]) + ").");
#endif

        }
        calculatedIndex += idx * strides[i];
        i++;
    }

    return data[calculatedIndex];
}

/**
 * Checks whether this array's contents approximately equal another one's contents
 * @param other the array to compare against
 * @param eps epsilon value for the absolute error
 * @return whether all the values of the arrays differ within eps
 */
template <typename T>
bool NDArray<T>::approxEquals(NDArray<T> other, double eps) {
    if (shape != other.shape || strides != other.strides) return false;

    bool areApproxEqual = true;
    for (int i = 0; i < data.size(); i++) {
        areApproxEqual &= diff(data[i], other.data[i]) < eps;
    }
    return areApproxEqual;
}

/**
 * Modifies the shape of this array
 * @param _dims the sizes per dimension; their product must amount to the same number of elements as the array before reshaping
 */
template<typename T>
template<SizeType ... S>
void NDArray<T>::reshape(S... _dims) {
    size_t numElements = 1;

    dims = sizeof...(_dims);
    shape = std::vector<size_t>();

    // https://stackoverflow.com/questions/28866559/writing-variadic-template-constructor
    (void)std::initializer_list<int>{(shape.push_back(std::forward<S>(_dims)), void(), 0)...};

    for (size_t i = 0; i < dims; i++)
        numElements *= shape[i];

    strides = std::vector<size_t>(dims);

    size_t curStride = 1;
    for (long long i = static_cast<long long>(dims) - 1; i >= 0; i--) {
        strides[i] = curStride;
        curStride *= shape[i];
    }
}
