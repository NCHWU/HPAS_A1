#pragma once

#include <iostream>

#include "FItSNE.h"


#define TEST_FOLDER "test_data"
#define TEST_NAME "mnist10k_step1"
#define NEGATIVE_GRADIENT_TEST_IDENTIFIER "negative_gradient"
#define LAGRANGE_INTERPOLATE_TEST_IDENTIFIER "lagrange_interpolate"
#define PRECALCULATE_LAGRANGE_DENOMINATOR_TEST_IDENTIFIER "precalculate_lagrange_denominator"
#define CALCULATE_INTERVALS_LOWER_BOUNDS_TEST_IDENTIFIER "calculate_intervals_lower_bounds"
#define CALCULATE_POINTS_RELATIVE_TO_GRID_TEST_IDENTIFIER "calculate_points_relative_to_grid"
#define CALCULATE_INTERVAL_INDICES_TEST_IDENTIFIER "calculate_interval_indices"
#define CALCULATE_POINTS_RELATIVE_TO_INTERVAL_TEST_IDENTIFIER "calculate_points_relative_to_interval"
#define CALCULATE_INTERPOLATION_POINTS_TEST_IDENTIFIER "calculate_interpolation_points"
#define CALCULATE_CHARGES_TEST_IDENTIFIER "calculate_charges"
#define CALCULATE_W_COEFFICIENTS_TEST_IDENTIFIER "calculate_wcoefficients"
#define CALCULATE_KERNEL_TEST_IDENTIFIER "calculate_kernel"
#define CALCULATE_POTENTIALS_QIJ_TEST_IDENTIFIER "calculate_potentials_qij"
#define VISUALIZE_W_COEFFICIENTS_TEST_IDENTIFIER "visualize_wcoefficients"
#define CALCULATE_SUM_Q_TEST_IDENTIFIER "calculate_sum_q"

#define SAVE_DATA(identifier, name, data) \
    writeToFile(data, RESOURCE_ROOT + std::string(TEST_FOLDER) + "/" + std::string(TEST_NAME) + "_" + std::string(identifier) + "_" + std::string(name) + ".bin")
#define READ_VEC(test_name, identifier, name) \
    readVec(RESOURCE_ROOT + std::string(TEST_FOLDER) + "/" + std::string(TEST_NAME) + "_" + std::string(identifier) + "_" + std::string(name) + ".bin")
#define READ_INT(test_name, identifier, name) \
    readInt(RESOURCE_ROOT + std::string(TEST_FOLDER) + "/" + std::string(TEST_NAME) + "_" + std::string(identifier) + "_" + std::string(name) + ".bin")
#define READ_PREC_FLOAT(test_name, identifier, name) \
    readPrecFloat(RESOURCE_ROOT + std::string(TEST_FOLDER) + "/" + std::string(TEST_NAME) + "_" + std::string(identifier) + "_" + std::string(name) + ".bin")
#define READ_FLOAT_NDARRAY(test_name, identifier, name) \
    NDArray<prec_float>::readFromFile(RESOURCE_ROOT + std::string(TEST_FOLDER) + "/" + std::string(TEST_NAME) + "_" + std::string(identifier) + "_" + std::string(name) + ".bin")
#define READ_VEC_NDARRAY(test_name, identifier, name) \
    NDArray<vec>::readFromFile(RESOURCE_ROOT + std::string(TEST_FOLDER) + "/" + std::string(TEST_NAME) + "_" + std::string(identifier) + "_" + std::string(name) + ".bin")
#define READ_IVEC_NDARRAY(test_name, identifier, name) \
    NDArray<glm::ivec2>::readFromFile(RESOURCE_ROOT + std::string(TEST_FOLDER) + "/" + std::string(TEST_NAME) + "_" + std::string(identifier) + "_" + std::string(name) + ".bin")


void writeToFile(NDArray<vec>& data, const std::string& filename)
{
	data.writeToFile(filename);
}
void writeToFile(NDArray<prec_float>& data, const std::string& filename)
{
    data.writeToFile(filename);
}
void writeToFile(NDArray<glm::ivec2>& data, const std::string& filename)
{
    data.writeToFile(filename);
}

template <typename T>
void writeToFile(T data, const std::string& filename)
{
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile)
    {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }

	outFile.write(reinterpret_cast<const char*>(&data), sizeof(T));

	outFile.close();
}
double readPrecFloat(const std::string& filename)
{
    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile)
    {
        throw std::runtime_error("Failed to open file for reading: " + filename);
    }

    double data;

    inFile.read(reinterpret_cast<char*>(&data), sizeof(double));

    return data;
}
int readInt(const std::string& filename)
{
    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile)
    {
        throw std::runtime_error("Failed to open file for reading: " + filename);
    }

	int data;

	inFile.read(reinterpret_cast<char*>(&data), sizeof(int));

	return data;
}
vec readVec(const std::string& filename)
{
    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile)
    {
        throw std::runtime_error("Failed to open file for reading: " + filename);
    }

	vec data;

	inFile.read(reinterpret_cast<char*>(&data), sizeof(vec));

    return data;
}

class WrappedFItSNE : public FItSNE
{
public:
    WrappedFItSNE() : FItSNE() {}

    void computeNegativeGradient(NDArray<vec>& points, NDArray<vec>& gradient, DebugRenderData& debugRenderData) override
    {
        SAVE_DATA(NEGATIVE_GRADIENT_TEST_IDENTIFIER, "points", points);
        SAVE_DATA(NEGATIVE_GRADIENT_TEST_IDENTIFIER, "gradient", gradient);
        FItSNE::computeNegativeGradient(points, gradient, debugRenderData);
		SAVE_DATA(NEGATIVE_GRADIENT_TEST_IDENTIFIER, "result", gradient);
    }

    bool ranLagrangeInterpolate{ false };
    vec lagrangeInterpolate(vec pointInInterval, int interpolationPointIndex, NDArray<prec_float>& lagrangeDenominator, NDArray<prec_float>& interpolationPointsInInterval, prec_float& min_coord, prec_float& max_coord) override
	{
        if (ranLagrangeInterpolate)
        {
            return FItSNE::lagrangeInterpolate(pointInInterval, interpolationPointIndex, lagrangeDenominator, interpolationPointsInInterval, min_coord, max_coord);
        }
		ranLagrangeInterpolate = true;
		SAVE_DATA(LAGRANGE_INTERPOLATE_TEST_IDENTIFIER, "pointInInterval", pointInInterval);
		SAVE_DATA(LAGRANGE_INTERPOLATE_TEST_IDENTIFIER, "interpolationPointIndex", interpolationPointIndex);
        SAVE_DATA(LAGRANGE_INTERPOLATE_TEST_IDENTIFIER, "lagrangeDenominator", lagrangeDenominator);
		SAVE_DATA(LAGRANGE_INTERPOLATE_TEST_IDENTIFIER, "interpolationPointsInInterval", interpolationPointsInInterval);
		SAVE_DATA(LAGRANGE_INTERPOLATE_TEST_IDENTIFIER, "min_coord", min_coord);
		SAVE_DATA(LAGRANGE_INTERPOLATE_TEST_IDENTIFIER, "max_coord", max_coord);
		vec result = FItSNE::lagrangeInterpolate(pointInInterval, interpolationPointIndex, lagrangeDenominator, interpolationPointsInInterval, min_coord, max_coord);
		SAVE_DATA(LAGRANGE_INTERPOLATE_TEST_IDENTIFIER, "result", result);
        return result;
    }

    bool ranPrecalculateLagrangeDenominator{ false };
    NDArray<prec_float> precalculateLagrangeDenominator(NDArray<prec_float>& interpolationPointsInInterval) override
    {
		if (ranPrecalculateLagrangeDenominator)
		{
			return FItSNE::precalculateLagrangeDenominator(interpolationPointsInInterval);
		}
		ranPrecalculateLagrangeDenominator = true;
		SAVE_DATA(PRECALCULATE_LAGRANGE_DENOMINATOR_TEST_IDENTIFIER, "interpolationPointsInInterval", interpolationPointsInInterval);
		NDArray<prec_float> result = FItSNE::precalculateLagrangeDenominator(interpolationPointsInInterval);
		SAVE_DATA(PRECALCULATE_LAGRANGE_DENOMINATOR_TEST_IDENTIFIER, "result", result);
		return result;
    }

    bool ranCalculateIntervalsLowerBounds{ false };
    NDArray<vec> calculateIntervalsLowerBounds(prec_float min_coord, prec_float max_coord) override
    {
        if (ranCalculateIntervalsLowerBounds)
        {
            return FItSNE::calculateIntervalsLowerBounds(min_coord, max_coord);
        }
		SAVE_DATA(CALCULATE_INTERVALS_LOWER_BOUNDS_TEST_IDENTIFIER, "min_coord", min_coord);
		SAVE_DATA(CALCULATE_INTERVALS_LOWER_BOUNDS_TEST_IDENTIFIER, "max_coord", max_coord);
        NDArray<vec> result = FItSNE::calculateIntervalsLowerBounds(min_coord, max_coord);
        SAVE_DATA(CALCULATE_INTERVALS_LOWER_BOUNDS_TEST_IDENTIFIER, "result", result);
        return result;
    }

    bool ranCalculatePointsRelativeToGrid{ false };
    NDArray<vec> calculatePointsRelativeToGrid(NDArray<vec>& points, prec_float min_coord, prec_float max_coord) override
    {
        if (ranCalculatePointsRelativeToGrid)
        {
            return FItSNE::calculatePointsRelativeToGrid(points, min_coord, max_coord);
        }
		SAVE_DATA(CALCULATE_POINTS_RELATIVE_TO_GRID_TEST_IDENTIFIER, "min_coord", min_coord);
		SAVE_DATA(CALCULATE_POINTS_RELATIVE_TO_GRID_TEST_IDENTIFIER, "max_coord", max_coord);
        SAVE_DATA(CALCULATE_POINTS_RELATIVE_TO_GRID_TEST_IDENTIFIER, "points", points);
        NDArray<vec> result = FItSNE::calculatePointsRelativeToGrid(points, min_coord, max_coord);
        SAVE_DATA(CALCULATE_POINTS_RELATIVE_TO_GRID_TEST_IDENTIFIER, "result", result);
        return result;
    }

    bool ranCalculateIntervalIndices{ false };
    NDArray<glm::ivec2> calculateIntervalIndices(NDArray<vec>& pointsRelativeToGrid) override
    {
        if (ranCalculateIntervalIndices)
        {
            return FItSNE::calculateIntervalIndices(pointsRelativeToGrid);
        }
        SAVE_DATA(CALCULATE_INTERVAL_INDICES_TEST_IDENTIFIER, "pointsRelativeToGrid", pointsRelativeToGrid);
        NDArray<glm::ivec2> result = FItSNE::calculateIntervalIndices(pointsRelativeToGrid);
        SAVE_DATA(CALCULATE_INTERVAL_INDICES_TEST_IDENTIFIER, "result", result);
        return result;
    }

    bool ranCalculatePointsRelativeToInterval{ false };
    NDArray<vec> calculatePointsRelativeToInterval(NDArray<vec>& pointsRelativeToGrid, NDArray<glm::ivec2>& intervalIndices) override
    {
        if (ranCalculatePointsRelativeToInterval)
        {
            return FItSNE::calculatePointsRelativeToInterval(pointsRelativeToGrid, intervalIndices);
        }
        SAVE_DATA(CALCULATE_POINTS_RELATIVE_TO_INTERVAL_TEST_IDENTIFIER, "pointsRelativeToGrid", pointsRelativeToGrid);
        SAVE_DATA(CALCULATE_POINTS_RELATIVE_TO_INTERVAL_TEST_IDENTIFIER, "intervalIndices", intervalIndices);
        NDArray<vec> result = FItSNE::calculatePointsRelativeToInterval(pointsRelativeToGrid, intervalIndices);
        SAVE_DATA(CALCULATE_POINTS_RELATIVE_TO_INTERVAL_TEST_IDENTIFIER, "result", result);
        return result;
    }

    bool ranCalculateInterpolationPoints{ false };
    NDArray<vec> calculateInterpolationPoints(NDArray<vec>& intervalsLowerBounds, prec_float gridWidth) override
    {
        if (ranCalculateInterpolationPoints)
        {
            return FItSNE::calculateInterpolationPoints(intervalsLowerBounds, gridWidth);
        }
        SAVE_DATA(CALCULATE_INTERPOLATION_POINTS_TEST_IDENTIFIER, "intervalsLowerBounds", intervalsLowerBounds);
        SAVE_DATA(CALCULATE_INTERPOLATION_POINTS_TEST_IDENTIFIER, "gridWidth", gridWidth);
        NDArray<vec> result = FItSNE::calculateInterpolationPoints(intervalsLowerBounds, gridWidth);
        SAVE_DATA(CALCULATE_INTERPOLATION_POINTS_TEST_IDENTIFIER, "result", result);
        return result;
    }

    bool ranCalculateCharges{ false };
    NDArray<double> calculateCharges(NDArray<vec>& points) override
    {
        if (ranCalculateCharges)
        {
            return FItSNE::calculateCharges(points);
        }
        SAVE_DATA(CALCULATE_CHARGES_TEST_IDENTIFIER, "points", points);
        NDArray<double> result = FItSNE::calculateCharges(points);
        SAVE_DATA(CALCULATE_CHARGES_TEST_IDENTIFIER, "result", result);
        return result;
    }

    bool ranCalculateWCoefficients{ false };
    NDArray<prec_float> calculateWCoefficients(
        NDArray<vec>& points,
        NDArray<glm::ivec2>& intervalIndices,
        NDArray<vec>& lagrangeInterpolatedValues,
        NDArray<prec_float>& charges) override
    {
        if (ranCalculateWCoefficients)
        {
            return FItSNE::calculateWCoefficients(points, intervalIndices, lagrangeInterpolatedValues, charges);
        }
        SAVE_DATA(CALCULATE_W_COEFFICIENTS_TEST_IDENTIFIER, "points", points);
        SAVE_DATA(CALCULATE_W_COEFFICIENTS_TEST_IDENTIFIER, "intervalIndices", intervalIndices);
        SAVE_DATA(CALCULATE_W_COEFFICIENTS_TEST_IDENTIFIER, "lagrangeInterpolatedValues", lagrangeInterpolatedValues);
        SAVE_DATA(CALCULATE_W_COEFFICIENTS_TEST_IDENTIFIER, "charges", charges);
        NDArray<prec_float> result = FItSNE::calculateWCoefficients(points, intervalIndices, lagrangeInterpolatedValues, charges);
        SAVE_DATA(CALCULATE_W_COEFFICIENTS_TEST_IDENTIFIER, "result", result);
        return result;
    }

    bool ranCalculateKernel{ false };
    NDArray<prec_float> calculateKernel(NDArray<vec>& interpolationPoints) override
    {
        if (ranCalculateKernel)
        {
            return FItSNE::calculateKernel(interpolationPoints);
        }
        SAVE_DATA(CALCULATE_KERNEL_TEST_IDENTIFIER, "interpolationPoints", interpolationPoints);
        NDArray<prec_float> result = FItSNE::calculateKernel(interpolationPoints);
        SAVE_DATA(CALCULATE_KERNEL_TEST_IDENTIFIER, "result", result);
        return result;
    }

    bool ranCalculatePotentialsQij{ false };
    NDArray<prec_float> calculatePotentialsQij(
        NDArray<vec>& points,
        NDArray<prec_float>& kernel,
        NDArray<prec_float>& w,
        NDArray<glm::ivec2>& intervalIndices,
        NDArray<vec>& lagrangeInterpolatedValues) override
    {
        if (ranCalculatePotentialsQij)
        {
            return FItSNE::calculatePotentialsQij(points, kernel, w, intervalIndices, lagrangeInterpolatedValues);
        }
        SAVE_DATA(CALCULATE_POTENTIALS_QIJ_TEST_IDENTIFIER, "points", points);
        SAVE_DATA(CALCULATE_POTENTIALS_QIJ_TEST_IDENTIFIER, "kernel", kernel);
        SAVE_DATA(CALCULATE_POTENTIALS_QIJ_TEST_IDENTIFIER, "w", w);
        SAVE_DATA(CALCULATE_POTENTIALS_QIJ_TEST_IDENTIFIER, "intervalIndices", intervalIndices);
        SAVE_DATA(CALCULATE_POTENTIALS_QIJ_TEST_IDENTIFIER, "lagrangeInterpolatedValues", lagrangeInterpolatedValues);
        NDArray<prec_float> result = FItSNE::calculatePotentialsQij(points, kernel, w, intervalIndices, lagrangeInterpolatedValues);
        SAVE_DATA(CALCULATE_POTENTIALS_QIJ_TEST_IDENTIFIER, "result", result);
        return result;
    }

    bool ranCalculateSumQ{ false };
    double calculateSumQ(NDArray<vec>& points, NDArray<prec_float>& potentialsQij) override
    {
        if (ranCalculateSumQ)
        {
            return FItSNE::calculateSumQ(points, potentialsQij);
        }
        SAVE_DATA(CALCULATE_SUM_Q_TEST_IDENTIFIER, "points", points);
        SAVE_DATA(CALCULATE_SUM_Q_TEST_IDENTIFIER, "potentialsQij", potentialsQij);
        double result = FItSNE::calculateSumQ(points, potentialsQij);
        SAVE_DATA(CALCULATE_SUM_Q_TEST_IDENTIFIER, "result", result);
        return result;
    }
};
