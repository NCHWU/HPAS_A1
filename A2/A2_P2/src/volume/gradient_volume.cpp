#include "gradient_volume.h"
#include <algorithm>
#include <exception>
#include <glm/geometric.hpp>
#include <glm/vector_relational.hpp>
#include <gsl/span>

namespace volume {

// Compute the maximum magnitude from all gradient voxels
static float computeMaxMagnitude(gsl::span<const GradientVoxel> data)
{
    return std::max_element(
        std::begin(data),
        std::end(data),
        [](const GradientVoxel& lhs, const GradientVoxel& rhs) {
            return lhs.magnitude < rhs.magnitude;
        })
        ->magnitude;
}

// Compute the minimum magnitude from all gradient voxels
static float computeMinMagnitude(gsl::span<const GradientVoxel> data)
{
    return std::min_element(
        std::begin(data),
        std::end(data),
        [](const GradientVoxel& lhs, const GradientVoxel& rhs) {
            return lhs.magnitude < rhs.magnitude;
        })
        ->magnitude;
}

// Compute a gradient volume from a volume
static std::vector<GradientVoxel> computeGradientVolume(const Volume& volume)
{
    const auto dim = volume.dims();

    std::vector<GradientVoxel> out(static_cast<size_t>(dim.x * dim.y * dim.z));
    for (int z = 1; z < dim.z - 1; z++) {
        for (int y = 1; y < dim.y - 1; y++) {
            for (int x = 1; x < dim.x - 1; x++) {
                const float gx = (volume.getVoxel(x + 1, y, z) - volume.getVoxel(x - 1, y, z)) / 2.0f;
                const float gy = (volume.getVoxel(x, y + 1, z) - volume.getVoxel(x, y - 1, z)) / 2.0f;
                const float gz = (volume.getVoxel(x, y, z + 1) - volume.getVoxel(x, y, z - 1)) / 2.0f;

                const glm::vec3 v { gx, gy, gz };
                const size_t index = static_cast<size_t>(x + dim.x * (y + dim.y * z));
                out[index] = GradientVoxel { v, glm::length(v) };
            }
        }
    }
    return out;
}

GradientVolume::GradientVolume(const Volume& volume)
    : m_dim(volume.dims())
    , m_data(computeGradientVolume(volume))
    , m_minMagnitude(computeMinMagnitude(m_data))
    , m_maxMagnitude(computeMaxMagnitude(m_data))
{
}

float GradientVolume::maxMagnitude() const
{
    return m_maxMagnitude;
}

float GradientVolume::minMagnitude() const
{
    return m_minMagnitude;
}

glm::ivec3 GradientVolume::dims() const
{
    return m_dim;
}

glm::vec4 GradientVolume::gradientToVec4(GradientVoxel voxel) const
{
    return glm::vec4(voxel.dir, voxel.magnitude);
}

// Convert list to vec4 to make it easier to convert to texture
std::vector<glm::vec4> GradientVolume::getVec4Data() const
{
    std::vector<glm::vec4> vec4List;
    vec4List.reserve(m_data.size()); // Reserve space for efficiency
    for (const auto& voxel : m_data) {
        vec4List.emplace_back(gradientToVec4(voxel));
    }
    return vec4List;
}

// This function returns a gradientVoxel at coord based on the current interpolation mode.
GradientVoxel GradientVolume::getGradientInterpolate(const glm::vec3& coord) const
{
    switch (interpolationMode) {
    case InterpolationMode::NearestNeighbour: {
        return getGradientNearestNeighbor(coord);
    }
    case InterpolationMode::Linear: {
        return getGradientLinearInterpolate(coord);
    }
    case InterpolationMode::Cubic: {
        // No cubic in this case, linear is good enough for the gradient.
        return getGradientLinearInterpolate(coord);
    }
    default: {
        throw std::exception();
    }
    };
}

// This function returns the nearest neighbour given a position in the volume given by coord.
// Notice that in this framework we assume that the distance between neighbouring voxels is 1 in all directions
GradientVoxel GradientVolume::getGradientNearestNeighbor(const glm::vec3& coord) const
{
    if (glm::any(glm::lessThan(coord, glm::vec3(0))) || glm::any(glm::greaterThanEqual(coord, glm::vec3(m_dim))))
        return { glm::vec3(0.0f), 0.0f };

    auto roundToPositiveInt = [](float f) {
        return static_cast<int>(f + 0.5f);
    };

    return getGradient(roundToPositiveInt(coord.x), roundToPositiveInt(coord.y), roundToPositiveInt(coord.z));
}

// ======= TODO : IMPLEMENT ========
// Returns the trilinearly interpolated gradinet at the given coordinate.
// Use the linearInterpolate function that you implemented below.
GradientVoxel GradientVolume::getGradientLinearInterpolate(const glm::vec3& coord) const
{
    // Get the surrounding voxel coordinates
    float x = coord.x;
    float y = coord.y;
    float z = coord.z;

    int x0 = static_cast<int>(std::floor(x));
    int y0 = static_cast<int>(std::floor(y));
    int z0 = static_cast<int>(std::floor(z));

    int x1 = x0 + 1;
    int y1 = y0 + 1;
    int z1 = z0 + 1;

    float dx = x - x0;
    float dy = y - y0;
    float dz = z - z0;

    // Check bounds
    if (x0 < 0 || y0 < 0 || z0 < 0 || x1 >= m_dim.x || y1 >= m_dim.y || z1 >= m_dim.z)
        return { glm::vec3(0.0f), 0.0f };

    // Get the surrounding gradient coordinates
    GradientVoxel g000 = getGradient(static_cast<int>(x0), static_cast<int>(y0), static_cast<int>(z0));
    GradientVoxel g100 = getGradient(static_cast<int>(x1), static_cast<int>(y0), static_cast<int>(z0));
    GradientVoxel g010 = getGradient(static_cast<int>(x0), static_cast<int>(y1), static_cast<int>(z0));
    GradientVoxel g110 = getGradient(static_cast<int>(x1), static_cast<int>(y1), static_cast<int>(z0));

    GradientVoxel g001 = getGradient(static_cast<int>(x0), static_cast<int>(y0), static_cast<int>(z1));
    GradientVoxel g101 = getGradient(static_cast<int>(x1), static_cast<int>(y0), static_cast<int>(z1));
    GradientVoxel g011 = getGradient(static_cast<int>(x0), static_cast<int>(y1), static_cast<int>(z1));
    GradientVoxel g111 = getGradient(static_cast<int>(x1), static_cast<int>(y1), static_cast<int>(z1));

    // Perform the trilinear interpolation
    GradientVoxel g00 = linearInterpolate(g000, g100, dx);
    GradientVoxel g10 = linearInterpolate(g010, g110, dx);
    GradientVoxel g01 = linearInterpolate(g001, g101, dx);
    GradientVoxel g11 = linearInterpolate(g011, g111, dx);

    GradientVoxel g0 = linearInterpolate(g00, g10, dy);
    GradientVoxel g1 = linearInterpolate(g01, g11, dy);

    GradientVoxel g = linearInterpolate(g0, g1, dz);

    return g;
}

// ======= TODO : IMPLEMENT ========
// This function should linearly interpolates the value from g0 to g1 given the factor (t).
// At t=0, linearInterpolate should return g0 and at t=1 it returns g1.
GradientVoxel GradientVolume::linearInterpolate(const GradientVoxel& g0, const GradientVoxel& g1, float factor)
{
    glm::vec3 resultDir = g0.dir * (1.0f - factor) + g1.dir * factor;
    float resultMagnitude = glm::length(resultDir);
    return GradientVoxel { resultDir, resultMagnitude };
}

// This function returns a gradientVoxel without using interpolation
GradientVoxel GradientVolume::getGradient(int x, int y, int z) const
{
    const size_t i = static_cast<size_t>(x + m_dim.x * (y + m_dim.y * z));
    return m_data[i];
}
}