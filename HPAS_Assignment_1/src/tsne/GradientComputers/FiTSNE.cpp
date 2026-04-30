#include "FItSNE.h"

FItSNE::FItSNE()
    : TSNEGradientCompute() {

    numInterpolationPoints = 4;
    numIntervals = 32;
}

/**
 ====== TODO: IMPLEMENT ======
 * Calculate the lagrange denominator for each interpolation point in the interval
 * @param interpolationPointsInInterval positions of points relative to their interval
 * @return the computed denominator value
 */
NDArray<prec_float> FItSNE::precalculateLagrangeDenominator(NDArray<prec_float>& interpolationPointsInInterval)
{
	START_TIMER();

    NDArray<prec_float> lagrangeDenominator = NDArray<prec_float>::empty({ (size_t)numInterpolationPoints });

    // Your code here

    LOG_TIME("Calculating lagrange denominator");
    return lagrangeDenominator;
}

/**
 ====== TODO: IMPLEMENT ======
 * Calculate the lower values of each interval's position in world space
 * @param min_coord minimum coordinate
 * @param max_coord maximum coordinate
 * @return the lower bounds of each interval
 */
NDArray<vec> FItSNE::calculateIntervalsLowerBounds(prec_float min_coord, prec_float max_coord)
{
    NDArray<vec> intervalsLowerBounds = NDArray<vec>::empty({ numIntervals, numIntervals });
    double intervalWidth = (max_coord - min_coord) / static_cast<double>(numIntervals);

	// Your code here

	return intervalsLowerBounds;
}

/**
 ====== TODO: IMPLEMENT ======
 * Calculate the positions of each point relative to the grid [0, 1]x[0, 1]
 * @param points
 * @param min_coord
 * @param max_coord
 * @return the positions of each point relative to the grid [0, 1]x[0, 1]
 */
NDArray<vec> FItSNE::calculatePointsRelativeToGrid(NDArray<vec>& points, prec_float min_coord, prec_float max_coord)
{
    NDArray<vec> pointsRelativeToGrid = NDArray<vec>::empty({ points.shape[0] });

	// Your code here

	return pointsRelativeToGrid;
}

/**
 ====== TODO: IMPLEMENT ======
 * Calculate the indices of each point's interval in the grid
 * @param pointsRelativeToGrid: the points relative to the grid [0, 1]x[0, 1]
**/
NDArray<glm::ivec2> FItSNE::calculateIntervalIndices(NDArray<vec>& pointsRelativeToGrid)
{
    const size_t numPoints = pointsRelativeToGrid.size();

    double relative_interval_width = 1.0f / static_cast<float>(numIntervals);

    NDArray<glm::ivec2> intervalIndices = NDArray<glm::ivec2>::empty({ numPoints });

	// Your code here

    return intervalIndices;
}

/**
 ====== TODO: IMPLEMENT ======
 * Calculate the positions of each point relative to its respective interval [0, 1]x[0, 1]
 * @param pointsRelativeToGrid the points relative to the interval [0, 1]x[0, 1]
 * @param intervalIndices the indices of the intervals to which the points belong
 * @return the position of each point relative to its interval
 */
NDArray<vec> FItSNE::calculatePointsRelativeToInterval(NDArray<vec>& pointsRelativeToGrid, NDArray<glm::ivec2>& intervalIndices)
{
    const size_t numPoints = pointsRelativeToGrid.size();

    double relative_interval_width = 1.0f / static_cast<float>(numIntervals);

    NDArray<vec> pointsRelativeToInterval = NDArray<vec>::empty({ numPoints });

    for (size_t i = 0; i < numPoints; i++)
    {
        vec curPointRelativeInGrid = pointsRelativeToGrid(i);
		vec intervalIndex = intervalIndices(i);

        // ====== TODO: IMPLEMENT ======
        // Calculate the positions of each point relative to its respective interval [0, 1]x[0, 1].
        // E.g. a point in the bottom left corner of its interval would have this positions be near (0, 0) 
        // and a point near the top right corner would have this position be near (1, 1).
        
        // pointsRelativeToInterval(i) = ...

        assert(pointsRelativeToInterval(i).x >= 0);
        assert(pointsRelativeToInterval(i).y >= 0);
        assert(pointsRelativeToInterval(i).x <= 1);
        assert(pointsRelativeToInterval(i).y <= 1);
    }

	return pointsRelativeToInterval;
}

/**
 ====== TODO: IMPLEMENT ======
 * Calculate the positions of each interpolation point in world space
 */
NDArray<vec> FItSNE::calculateInterpolationPoints(NDArray<vec>& intervalsLowerBounds, prec_float gridWidth)
{
	NDArray<vec> interpolationPoints = NDArray<vec>::empty({ numIntervals * numInterpolationPoints, numIntervals * numInterpolationPoints });
    double h = gridWidth / (numIntervals * numInterpolationPoints);

    for (size_t y = 0; y < numIntervals; y++)
    {
        for (size_t x = 0; x < numIntervals; x++)
        {
            for (size_t py = 0; py < numInterpolationPoints; py++)
            {
                for (size_t px = 0; px < numInterpolationPoints; px++)
                {
                    vec y_tilde;

					// ====== TODO: IMPLEMENT ======
					// Calculate the positions of each interpolation point in world space (y_tilde)
					// h is the distance between each interpolation point in world space
					// Keep in mind that the interpolation points are centered in the interval,
                    // and none are on the interval boundary.

                    // y_tilde = ...

                    interpolationPoints(y * numInterpolationPoints + py, x * numInterpolationPoints + px) = y_tilde;
                }
            }
        }
    }

	return interpolationPoints;
}

/**
 ======= TODO: IMPLEMENT ========
 * Calculate the charges Qij for each point
 * The charges are calculated as follows:
 * charge Q_i = [1, x, y, x^2 + y^2]
 * where x and y are the coordinates of the point
 * @param points: the points for which to calculate the charges
*/
NDArray<double> FItSNE::calculateCharges(NDArray<vec>& points)
{
    const size_t numPoints = points.size();

    NDArray<double> charges = NDArray<double>::empty({ points.size(), (size_t)n_terms });

    // Compute the charges for each point here

	return charges;
}

/**
 * Calculate the lagrange coefficients for each interpolation point
 */
NDArray<prec_float> FItSNE::calculateWCoefficients(
    NDArray<vec>& points,
    NDArray<glm::ivec2>& intervalIndices,
    NDArray<vec>& lagrangeInterpolatedValues,
    NDArray<prec_float>& charges)
{
	START_TIMER();

    const size_t numPoints = points.size();
    NDArray<double> w = NDArray<double>::empty({ numIntervals * numInterpolationPoints, numIntervals * numInterpolationPoints, (size_t)n_terms });

#pragma omp parallel for
    for (int i = 0; i < numPoints; i++)
    {
        glm::ivec2 intervalIdx = intervalIndices(i);

        for (size_t py = 0; py < numInterpolationPoints; py++)
        {
            for (size_t px = 0; px < numInterpolationPoints; px++)
            {
                // Retrieve the previously calculated lagrange value
                prec_float lagrangeTermX = lagrangeInterpolatedValues(i, px).x;
                prec_float lagrangeTermY = lagrangeInterpolatedValues(i, py).y;
                prec_float lagrangeTerm = lagrangeTermX * lagrangeTermY;


                for (size_t term = 0; term < n_terms; term++)
                {
                    prec_float q_i = charges(i, term);
#pragma omp critical
                    w(
                        intervalIdx.x * numInterpolationPoints + px,
                        intervalIdx.y * numInterpolationPoints + py,
                        term
                    )
                        += lagrangeTerm * q_i;
                }
            }
        }

    }

    LOG_TIME("Calculating the w coefficients");

    return w;
}
/**
 * Calculate the kernel tilde
 * @param interpolationPoints: 2D array of size (numIntervals * numInterpolationPoints)^2, containing
   the points for which to calculate the kernel
**/
NDArray<prec_float> FItSNE::calculateKernel(NDArray<vec>& interpolationPoints)
{
	START_TIMER();

    NDArray<prec_float> kernel_tilde = NDArray<prec_float>::empty({ 2 * numIntervals * numInterpolationPoints, 2 * numIntervals * numInterpolationPoints });

#pragma omp parallel for
    for (int y = 0; y < numIntervals * numInterpolationPoints; y++)
    {
        for (size_t x = 0; x < numIntervals * numInterpolationPoints; x++)
        {
            prec_float tmp = cauchyKernelSquared(interpolationPoints(0, 0), interpolationPoints(y, x));

            kernel_tilde(numIntervals * numInterpolationPoints + y, numIntervals * numInterpolationPoints + x) = tmp;
            kernel_tilde(numIntervals * numInterpolationPoints - y, numIntervals * numInterpolationPoints + x) = tmp;
            kernel_tilde(numIntervals * numInterpolationPoints + y, numIntervals * numInterpolationPoints - x) = tmp;
            kernel_tilde(numIntervals * numInterpolationPoints - y, numIntervals * numInterpolationPoints - x) = tmp;
        }
    }

    LOG_TIME("Creating the kernel tilde");

	return kernel_tilde;
}

NDArray<prec_float> FItSNE::calculatePotentialsQij(
    NDArray<vec>& points,
    NDArray<prec_float>& kernel,
    NDArray<prec_float>& w,
    NDArray<glm::ivec2>& intervalIndices,
    NDArray<vec>& lagrangeInterpolatedValues)
{
    START_TIMER();

    const size_t numPoints = points.size();

	// Compute the fft of the kernel
    NDArray<std::complex<double>> kernel_fft = fft_2d(kernel);

    LOG_TIME("Finding the fft of the kernel tilde");

    NDArray<double> potentialsQij = NDArray<double>::empty({ numPoints, (size_t)n_terms });

#pragma omp parallel for
    for (int term = 0; term < n_terms; term++)
    {
        // Pad the w to make it fit with the kernel that's twice as big
        NDArray<double> w_padded = NDArray<double>::empty({ 2 * numIntervals * numInterpolationPoints, 2 * numIntervals * numInterpolationPoints });

#pragma omp parallel for
        for (int y = 0; y < numIntervals * numInterpolationPoints; y++)
        {
            for (size_t x = 0; x < numIntervals * numInterpolationPoints; x++)
            {
                w_padded(y, x) = w(y, x, term);
            }
        }

        LOG_TIME("Padding w");

        NDArray<std::complex<double>> w_fft = fft_2d(w_padded);

        LOG_TIME("Finding the fft of w");

        // For multiplication, these must have the same shape
        assert(w_fft.shape[0] == kernel_fft.shape[0]);
        assert(w_fft.shape[1] == kernel_fft.shape[1]);

        /*
         ======= TODO: IMPLEMENT ========
         * Multiply the FFT of the kernel with the FFT of w using the Hadamard product
         */

        // Multiply the ffts together
        NDArray<std::complex<double>> result_fft = NDArray<std::complex<double>>::empty({ w_fft.shape[0], w_fft.shape[1] });



        LOG_TIME("Multiplying the ffts");

        NDArray<double> result = ifft_2d(result_fft);

        LOG_TIME("Inverse fft");

        /*
		 * Compute the potentials based on the lagrange coefficients and the result from the FFT operations
         */

#pragma omp parallel for
        for (int i = 0; i < numPoints; i++)
        {
			glm::ivec2 intervalIdx = intervalIndices(i);

            for (size_t py = 0; py < numInterpolationPoints; py++)
            {
                for (size_t px = 0; px < numInterpolationPoints; px++)
                {
                    // Retrieve the previously calculated lagrange value
                    prec_float lagrangeTermX = lagrangeInterpolatedValues(i, px).x;
                    prec_float lagrangeTermY = lagrangeInterpolatedValues(i, py).y;
                    prec_float lagrangeTerm = lagrangeTermX * lagrangeTermY;

                    prec_float result_value = result(
                        numIntervals * numInterpolationPoints + intervalIdx.x * numInterpolationPoints + px,
                        numIntervals * numInterpolationPoints + intervalIdx.y * numInterpolationPoints + py
                    );

#pragma omp critical
                    potentialsQij(
                        i,
                        term
                    )
                        += lagrangeTerm * result_value;
                }
            }
        }
    }

    LOG_TIME("Calculating the potentials");
	return potentialsQij;
}

void FItSNE::visualizeWCoefficients(NDArray<prec_float>& wCoefficients, NDArray<vec>& interpolationPoints, DebugRenderData& debugRenderData)
{
	for (size_t y = 0; y < numIntervals * numInterpolationPoints; y++)
	{
		for (size_t x = 0; x < numIntervals * numInterpolationPoints; x++)
		{
			vec point = interpolationPoints(y, x);
            prec_float t0 = wCoefficients(y, x, 0);
            prec_float t1 = wCoefficients(y, x, 1);
            prec_float t2 = wCoefficients(y, x, 2);
            prec_float t3 = wCoefficients(y, x, 3);
			debugRenderData.addLine(point, point + vec(-1, 0) * t0 * 0.001, { 0, 0, 1 });
            debugRenderData.addLine(point, point + vec(0, 1) * t1 * 0.001, { 0, 1, 0 });
            debugRenderData.addLine(point, point + vec(1, 0) * t2 * 0.001, { 0, 1, 0 });
            debugRenderData.addLine(point, point + vec(0, -1) * t3 * 0.001, { 0, 1, 0 });
		}
	}
}

/**
 ======= TODO: IMPLEMENT ========
 * Calculate the sum of charges
 * @param points: the points for which to calculate the sum of charges
 * @param potentialsQij: the potentials for each point
 * @return the sum of charges
**/
double FItSNE::calculateSumQ(NDArray<vec>& points, NDArray<prec_float>& potentialsQij)
{
	START_TIMER();

    double sum_Q = 0;
	const size_t numPoints = points.size();

	// Compute the sum of charges here

    LOG_TIME("Calculating SumQ");

	return sum_Q;
}

vec FItSNE::lagrangeInterpolate(vec pointInInterval, int interpolationPointIndex,
    NDArray<prec_float>& lagrangeDenominator, NDArray<prec_float>& interpolationPointsInInterval, prec_float& min_coord, prec_float& max_coord) {

    vec numerator = vec(1);
    for (size_t y = 0; y < numInterpolationPoints; y++) {
        if (y == interpolationPointIndex)
        {
            continue;
        }
        prec_float local_point_y = pointInInterval.y;
        numerator.y *= local_point_y - interpolationPointsInInterval(y);
    }

    for (size_t x = 0; x < numInterpolationPoints; x++)
    {
        if (x == interpolationPointIndex)
        {
            continue;
        }
        prec_float local_point_x = pointInInterval.x;
        numerator.x *= local_point_x - interpolationPointsInInterval(x);
    }

    return numerator / lagrangeDenominator(interpolationPointIndex);
}

void FItSNE::precalculate_q_denom(NDArray<vec>& points, DebugRenderData& debugRenderData)
{}

/**
  TODO: Implement the negative force calculation here
  Compute the negative gradient of the points in the TSNE algorithm.
**/
void FItSNE::computeNegativeGradient(NDArray<vec>& points, NDArray<vec>& gradient, DebugRenderData& debugRenderData) {
    const size_t numPoints = points.size();

    START_TIMER();

    /*
     * 1. Compute squares in the grid & evaluate kernel at interpolation points
     */

    // Find min/max bounds for the interpolation grid
    prec_float min_coord = std::numeric_limits<prec_float>::max();
    prec_float max_coord = std::numeric_limits<prec_float>::min();

    for (unsigned int i = 0; i < numPoints; i++) {
        min_coord = std::min(min_coord, points(i).x);
        min_coord = std::min(min_coord, points(i).y);
        max_coord = std::max(max_coord, points(i).x);
        max_coord = std::max(max_coord, points(i).y);
    }

    LOG_TIME("Finding min and max coords");

    // Compute lower bound of intervals
	NDArray<vec> intervalsLowerBounds = calculateIntervalsLowerBounds(min_coord, max_coord);

    // Calculate the local positions of the points within the grid
	NDArray<vec> pointsRelativeToGrid = calculatePointsRelativeToGrid(points, min_coord, max_coord);

	// Calculate the 2D index of the interval that each point belongs to
	NDArray<glm::ivec2> intervalIndices = calculateIntervalIndices(pointsRelativeToGrid);

    // Calculate the local positions of the points within their respective intervals
	NDArray<vec> pointsRelativeToInterval = calculatePointsRelativeToInterval(pointsRelativeToGrid, intervalIndices);


    LOG_TIME("Computing interval and point positions");



    // Draw the interpolation grid
    for (size_t i = 0; i < numIntervals; i++) {
        vec b = intervalsLowerBounds(i, numIntervals - 1);
        vec a = intervalsLowerBounds(numIntervals - 1, i);
        debugRenderData.addLine(vec(min_coord, a.y), vec(max_coord, a.y), {1, 0, 0});
        debugRenderData.addLine(vec(b.x, min_coord), vec(b.x, max_coord), {0, 0, 1});
    }
    debugRenderData.addLine(vec(min_coord, max_coord), vec(max_coord, max_coord), {1, 0, 0});
    debugRenderData.addLine(vec(max_coord, min_coord), vec(max_coord, max_coord), {0, 0, 1});

    LOG_TIME("Drawing interpolation grid to debug");

	// Compute the location of each interpolation point in world space
	NDArray<vec> interpolationPoints = calculateInterpolationPoints(intervalsLowerBounds, max_coord - min_coord);

    double relative_interval_width = 1.0f / static_cast<float>(numIntervals);

    NDArray<prec_float> interpolationPointsInInterval = NDArray<prec_float>::empty({ (size_t)numInterpolationPoints }); // like y_tilde_spacings
    double h = 1.0 / (numIntervals * numInterpolationPoints);
    for (size_t i = 0; i < numInterpolationPoints; i++)
    {
        double y_tilde = h / 2 + h * i; // this is relative position! (to the interval)
        interpolationPointsInInterval(i) = y_tilde / relative_interval_width;
    }



    LOG_TIME("Drawing debug data");

	NDArray<double> charges = calculateCharges(points);

    NDArray<prec_float> lagrangeDenominator = precalculateLagrangeDenominator(interpolationPointsInInterval);

    NDArray<vec> lagrangeInterpolatedValues = NDArray<vec>::empty({ numPoints, (size_t)numInterpolationPoints });

#pragma omp parallel for
    for (int i = 0; i < numPoints; i++) {
        vec currentPointRelativeToInterval = pointsRelativeToInterval(i);

        for (size_t j = 0; j < numInterpolationPoints; j++) {
            lagrangeInterpolatedValues(i, j) = lagrangeInterpolate(currentPointRelativeToInterval, j, lagrangeDenominator, interpolationPointsInInterval, min_coord, max_coord);
        }
    }

    LOG_TIME("Calculating the lagrange coefficients");


    // Compute the w coefficients
	NDArray<double> w = calculateWCoefficients(points, intervalIndices, lagrangeInterpolatedValues, charges);
	//visualizeWCoefficients(w, interpolationPoints, debugRenderData);

    // Compute the Toeplitz kernel
	NDArray<double> kernel_tilde = calculateKernel(interpolationPoints);

    LOG_TIME("Creating the kernel tilde");

    NDArray<double> potentialsQij = calculatePotentialsQij(points, kernel_tilde, w, intervalIndices, lagrangeInterpolatedValues);

    double sum_Q = calculateSumQ(points, potentialsQij);

	q_denom_precalculated = sum_Q;

#pragma omp parallel for
    for (int i = 0; i < numPoints; i++)
    {
        vec point = points(i);
        vec negativeForce;

        /*
         * TODO: Implement the negative force calculation here
         */

		gradient(i) += 4.0 * negativeForce;
    }

    LOG_TIME("Applying the gradient");
}
