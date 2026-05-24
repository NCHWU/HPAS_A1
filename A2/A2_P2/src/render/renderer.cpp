#include "renderer.h"
#include <algorithm>
#include <algorithm> // std::fill
#include <cmath>
#include <functional>
#include <glm/common.hpp>
#include <glm/gtx/component_wise.hpp>
#include <iostream>
#include <tuple>

namespace render {

// The renderer is passed a pointer to the volume, gradinet volume, camera and an initial renderConfig.
// The camera being pointed to may change each frame (when the user interacts). When the renderConfig
// changes the setConfig function is called with the updated render config. This gives the Renderer an
// opportunity to resize the framebuffer.
Renderer::Renderer(
    const volume::Volume* pVolume,
    const volume::GradientVolume* pGradientVolume,
    const render::RayTraceCamera* pCamera,
    const RenderConfig& initialConfig)
    : m_pVolume(pVolume)
    , m_pGradientVolume(pGradientVolume)
    , m_pCamera(pCamera)
    , m_config(initialConfig)
{
    resizeImage(initialConfig.renderResolution);
}

// Set a new render config if the user changed the settings.
void Renderer::setConfig(const RenderConfig& config)
{
    if (config.renderResolution != m_config.renderResolution)
        resizeImage(config.renderResolution);

    m_config = config;
}

// Resize the framebuffer and fill it with black pixels.
void Renderer::resizeImage(const glm::ivec2& resolution)
{
    m_frameBuffer.resize(size_t(resolution.x) * size_t(resolution.y), glm::vec4(0.0f));
}

// Clear the framebuffer by setting all pixels to black.
void Renderer::resetImage()
{
    std::fill(std::begin(m_frameBuffer), std::end(m_frameBuffer), glm::vec4(0.0f));
}

// Return a VIEW into the framebuffer. This view is merely a reference to the m_frameBuffer member variable.
// This does NOT make a copy of the framebuffer.
gsl::span<const glm::vec4> Renderer::frameBuffer() const
{
    return m_frameBuffer;
}

// Main render function. It computes an image according to the current renderMode.
// Multithreading is enabled in Release/RelWithDebInfo modes. In Debug mode multithreading is disabled to make debugging easier.
void Renderer::render()
{
    resetImage();

    const glm::vec3 planeNormal = -glm::normalize(m_pCamera->forward());
    const glm::vec3 volumeCenter = glm::vec3(m_pVolume->dims()) / 2.0f;
    const Bounds bounds { glm::vec3(0.0f), glm::vec3(m_pVolume->dims() - glm::ivec3(1)) };

    // 0 = sequential (single-core), 1 = OMP (multi-core)
#ifdef NDEBUG
    // If NOT in debug mode then enable parallelism using the Open MP
#define PARALLELISM 1
#else
    // Disable multi threading in debug mode.
#define PARALLELISM 0
#endif

#if PARALLELISM == 1
#pragma omp parallel for
#endif
    for (int x = 0; x < m_config.renderResolution.x; x++) {
        for (int y = 0; y < m_config.renderResolution.y; y++) {

            // Compute a ray for the current pixel.
            const glm::vec2 pixelPos = glm::vec2(x, y) / glm::vec2(m_config.renderResolution);
            Ray ray = m_pCamera->generateRay(pixelPos * 2.0f - 1.0f);

            // Compute where the ray enters and exists the volume.
            // If the ray misses the volume then we continue to the next pixel.
            if (!instersectRayVolumeBounds(ray, bounds))
                continue;

            // Get a color for the current pixel according to the current render mode.
            glm::vec4 color {};
            switch (m_config.renderMode) {
            case RenderMode::RenderSlicer: {
                color = traceRaySlice(ray, volumeCenter, planeNormal);
                break;
            }
            case RenderMode::RenderMIP: {
                color = traceRayMIP(ray, m_config.stepSize);
                break;
            }
            case RenderMode::RenderComposite: {
                color = traceRayComposite(ray, m_config.stepSize);
                break;
            }
            case RenderMode::RenderIso: {
                color = traceRayISO(ray, m_config.stepSize);
                break;
            }
            };
            // Write the resulting color to the screen.
            fillColor(x, y, color);
        }
    }
}

// ======= DO NOT MODIFY THIS FUNCTION ========
// This function generates a view alongside a plane perpendicular to the camera through the center of the volume
//  using the slicing technique.
glm::vec4 Renderer::traceRaySlice(const Ray& ray, const glm::vec3& volumeCenter, const glm::vec3& planeNormal) const
{
    const float t = glm::dot(volumeCenter - ray.origin, planeNormal) / glm::dot(ray.direction, planeNormal);
    const glm::vec3 samplePos = ray.origin + ray.direction * t;
    const float val = m_pVolume->getSampleInterpolate(samplePos);
    return glm::vec4(glm::vec3(std::max(val / m_pVolume->maximum(), 0.0f)), 1.f);
}

// ======= DO NOT MODIFY THIS FUNCTION ========
// Function that implements maximum-intensity-projection (MIP) raycasting.
// It returns the color assigned to a ray/pixel given it's origin, direction and the distances
// at which it enters/exits the volume (ray.tmin & ray.tmax respectively).
// The ray must be sampled with a distance defined by the stepSize
glm::vec4 Renderer::traceRayMIP(const Ray& ray, float stepSize) const
{
    float maxVal = 0.0f;

    // Incrementing samplePos directly instead of recomputing it each frame gives a measureable speed-up.
    glm::vec3 samplePos = ray.origin + ray.tmin * ray.direction;
    const glm::vec3 increment = stepSize * ray.direction;
    for (float t = ray.tmin; t <= ray.tmax; t += stepSize, samplePos += increment) {
        const float val = m_pVolume->getSampleInterpolate(samplePos);
        maxVal = std::max(val, maxVal);
    }

    // Normalize the result to a range of [0 to mpVolume->maximum()].
    return glm::vec4(glm::vec3(maxVal) / m_pVolume->maximum(), 1.0f);
}

// ======= TODO: IMPLEMENT ========
//
// Assignment 1.
//
// This function should find the position where the ray intersects with the volume's isosurface.
// If volume shading is DISABLED then simply return the isoColor.
// If volume shading is ENABLED then return the phong-shaded color at that location using the local gradient (from m_pGradientVolume).
//   Use the camera position (m_pCamera->position()) as the light position.
// Use the bisectionAccuracy function (to be implemented) to get a more precise isosurface location between two steps.
glm::vec4 Renderer::traceRayISO(const Ray& ray, float stepSize) const
{
    float isoval = m_config.isoValue;
    static constexpr glm::vec3 isoColor { 0.8f, 0.8f, 0.2f };
    static constexpr glm::vec3 background { 0.0f, 0.0f, 0.0f };

    // Incrementing samplePos directly instead of recomputing it each frame gives a measureable speed-up.
    glm::vec3 samplePos = ray.origin + ray.tmin * ray.direction;
    const glm::vec3 increment = stepSize * ray.direction;
    float prev_Intensity = m_pVolume->getSampleInterpolate(samplePos);

    for (float t = ray.tmin; t <= ray.tmax; t += stepSize, samplePos += increment) {

        const float curr_Intensity = m_pVolume->getSampleInterpolate(samplePos);

        // Check if the isovalue lies between the previous and current intensity values. If it does then we have found an intersection with the isosurface.
        if (isoval > prev_Intensity && isoval <= curr_Intensity) {

            glm::vec3 hitPos = samplePos;
            // If bisection is enabled then we can use the bisection method to find a more precise intersection point between the previous and current sample positions.
            if (m_config.bisection) {

                // We can call the bisectionAccuracy function with the ray, the previous and current t values and the iso value to get a more precise t value for the intersection point.
                float t0 = std::max(t - stepSize, ray.tmin);
                float t1 = t;
                float t_bisect = bisectionAccuracy(ray, t0, t1, isoval);

                // We compute the sample position corresponding to this t_bisect value and use it for shading.
                hitPos = ray.origin + t_bisect * ray.direction;
            }

            // If shading is disabled then we can simply return the isoColor.
            if (!m_config.volumeShading)
                return glm::vec4(isoColor, 1.0f);

            // If shading is enabled then we need to compute the Phong shading at the intersection point.
            // We can use the gradient at the intersection point as the normal vector for the Phong shading model.
            const volume::GradientVoxel gradient = m_pGradientVolume->getGradientInterpolate(hitPos);
            const glm::vec3 lightPos = m_pCamera->position();

            // Compute the light vector (L) and view vector (V) for the Phong shading model.
            // The light vector is the direction from the intersection point to the light position and the view vector is the direction from the intersection point to the camera position.
            const glm::vec3 L = lightPos - hitPos;
            const glm::vec3 V = m_pCamera->position() - hitPos;

            // Compute the Phong shading at the intersection point using the isoColor as the material color, the gradient as the normal vector, and the light and view vectors.
            const glm::vec3 shadedColor = computePhongShading(isoColor, gradient, L, V);
            return glm::vec4(shadedColor, 1.0f);
        }
        prev_Intensity = curr_Intensity;
    }

    return glm::vec4(background, 1.0f);
}

// ======= TODO: IMPLEMENT ========
//
// Assignment 1.
//
// Given that the iso value lies somewhere between t0 and t1, find a t for which the value
// closely matches the iso value (less than 0.01 difference). Add a limit to the number of
// iterations such that it does not get stuck in degerate cases.
float Renderer::bisectionAccuracy(const Ray& ray, float t0, float t1, float isoValue) const
{
    const float epsilon = 0.01f;
    const int maxIterations = 20;

    for (int i = 0; i < maxIterations; i++) {
        // Calculate mid point between t0 and t1
        float t_mid = (t0 + t1) / 2.0f;

        // Calculate the position corresponding to t_mid.
        glm::vec3 pos_mid = ray.origin + t_mid * ray.direction;

        // Calculate the value at the mid point using trilinear interpolation.
        float val_mid = m_pVolume->getSampleInterpolate(pos_mid);

        // Check if the value at the mid point is close enough to the iso value. If it is then we can return t_mid as the result.
        if (std::abs(isoValue - val_mid) < epsilon)
            return t_mid;

        // If the value at the mid point is less than the iso value then we know that the iso value lies between t_mid and t1. Otherwise, it lies between t0 and t_mid. We can then update t0 or t1 accordingly and repeat the process until we find a t for which the value closely matches the iso value or we reach the maximum number of iterations.
        if (isoValue < val_mid)
            t1 = t_mid;
        else
            t0 = t_mid;
    }

    return 0.5f * (t0 + t1);
}

// ======= TODO: IMPLEMENT ========
//
// Assignment 1.
//
// Compute Phong Shading given the voxel color (material color), the gradient, the light vector and view vector.
// You can find out more about the Phong shading model at:
// https://en.wikipedia.org/wiki/Phong_reflection_model
//
// you are allowed to modify the default parameters in the .h file, if needed. DO NOT modify them inside the function.

glm::vec3 Renderer::computePhongShading(const glm::vec3& color, const volume::GradientVoxel& gradient, const glm::vec3& L, const glm::vec3& V, float ambientCoefficient, float diffuseCoefficient, float specularCoefficient, int specularPower)
{
    // Gradient magnitude is used as a measure of how much light is reflected by the surface. If the gradient magnitude is very small then we can assume that the surface is not reflecting any light and we can return the ambient color.
    if (gradient.magnitude < 1e-6f)
        return ambientCoefficient * color;

    // Normalize the gradient direction, light vector and view vector to get the normal vector, light direction and view direction respectively.
    glm::vec3 N = glm::normalize(gradient.dir);
    glm::vec3 l = glm::normalize(L);
    glm::vec3 v = glm::normalize(V);

    // If the dot product of the normal vector and the light direction is negative then we need to flip the normal vector to ensure that it is facing towards the light source.
    // This is because the Phong shading model assumes that the normal vector is facing towards the light source.
    if (glm::dot(N, l) < 0.0f)
        N = -N;

    // Calculate the ambient, diffuse and specular components of the Phong shading model and return the final color.
    // The ambient component is simply the ambient coefficient multiplied by the material color.
    glm::vec3 ambient = ambientCoefficient * color;

    // The diffuse component is the diffuse coefficient multiplied by the material color and the dot product of the normal vector and the light direction. We also take the max with 0 to ensure that we don't have negative lighting.
    glm::vec3 diffuse = diffuseCoefficient * color * std::max(glm::dot(N, l), 0.0f);

    // The specular component is the specular coefficient multiplied by the dot product of the reflected light direction and the view direction raised to the power of the specular power.
    // The reflected light direction is calculated using the reflect function from glm which reflects the light vector around the normal vector. We also take the max with 0 to ensure that we don't have negative lighting.
    glm::vec3 R = glm::reflect(-l, N);
    float specpower = std::pow(glm::max(glm::dot(glm::normalize(R), v), 0.0f), specularPower);

    // We can also multiply the specular component by the material color to get a colored specular highlight.
    // This is not physically accurate but it can give a more visually appealing result.
    glm::vec3 specular = specularCoefficient * specpower * glm::vec3(1.0f);

    glm::vec3 resultColor = ambient + diffuse + specular;

    // Clamp the resulting color to a range of [0, 1] to ensure that we don't have any color values that are outside the valid range.
    resultColor = glm::clamp(resultColor, glm::vec3(0.0f), glm::vec3(1.0f));
    return resultColor;
}

// ======= TODO: IMPLEMENT ========
//
// Assignment 1.
//
// In this function, implement 1D transfer function raycasting.
// Use getTFValue to compute the color for a given volume value according to the 1D transfer function.
glm::vec4 Renderer::traceRayComposite(const Ray& ray, float stepSize) const
{
    glm::vec3 accumulatedColor(0.0f);
    float accumulatedOpacity = 0.0f;

    // Implementation of front-to-back compositing.
    glm::vec3 samplePos = ray.origin + ray.tmin * ray.direction;
    const glm::vec3 increment = stepSize * ray.direction;
    for (float t = ray.tmin; t <= ray.tmax; t += stepSize, samplePos += increment) {

        // Get the volume value at the current sample position and use the transfer function to get the corresponding color and opacity.
        float val = m_pVolume->getSampleInterpolate(samplePos);
        glm::vec4 tfColor = getTFValue(val);

        // Early termination if opacity is close to 1.
        if (accumulatedOpacity >= 0.99f)
            break;

        // Front-to-back compositing formula: C_acc = C_acc + (1 - alpha_acc) * C_sample * alpha_sample
        accumulatedColor += (1.0f - accumulatedOpacity) * tfColor.a * glm::vec3(tfColor);

        // Update the accumulated opacity using the formula: alpha_acc = alpha_acc + (1 - alpha_acc) * alpha_sample
        accumulatedOpacity += (1.0f - accumulatedOpacity) * tfColor.a;
    }
    return glm::vec4(accumulatedColor, accumulatedOpacity);
}

// ======= DO NOT MODIFY THIS FUNCTION ========
// Looks up the color+opacity corresponding to the given volume value from the 1D tranfer function LUT (m_config.tfColorMap).
// The value will initially range from (m_config.tfColorMapIndexStart) to (m_config.tfColorMapIndexStart + m_config.tfColorMapIndexRange) .
glm::vec4 Renderer::getTFValue(float val) const
{
    // Map value from [m_config.tfColorMapIndexStart, m_config.tfColorMapIndexStart + m_config.tfColorMapIndexRange) to [0, 1) .
    const float range01 = (val - m_config.tfColorMapIndexStart) / m_config.tfColorMapIndexRange;
    const size_t i = std::min(static_cast<size_t>(range01 * static_cast<float>(m_config.tfColorMap.size())), m_config.tfColorMap.size() - 1);
    return m_config.tfColorMap[i];
}

// This function computes if a ray intersects with the axis-aligned bounding box around the volume.
// If the ray intersects then tmin/tmax are set to the distance at which the ray hits/exists the
// volume and true is returned. If the ray misses the volume the the function returns false.
//
// If you are interested you can learn about it at.
// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
bool Renderer::instersectRayVolumeBounds(Ray& ray, const Bounds& bounds) const
{
    const glm::vec3 invDir = 1.0f / ray.direction;
    const glm::bvec3 sign = glm::lessThan(invDir, glm::vec3(0.0f));

    float tmin = (bounds.lowerUpper[sign[0]].x - ray.origin.x) * invDir.x;
    float tmax = (bounds.lowerUpper[!sign[0]].x - ray.origin.x) * invDir.x;
    const float tymin = (bounds.lowerUpper[sign[1]].y - ray.origin.y) * invDir.y;
    const float tymax = (bounds.lowerUpper[!sign[1]].y - ray.origin.y) * invDir.y;

    if ((tmin > tymax) || (tymin > tmax))
        return false;
    tmin = std::max(tmin, tymin);
    tmax = std::min(tmax, tymax);

    const float tzmin = (bounds.lowerUpper[sign[2]].z - ray.origin.z) * invDir.z;
    const float tzmax = (bounds.lowerUpper[!sign[2]].z - ray.origin.z) * invDir.z;

    if ((tmin > tzmax) || (tzmin > tmax))
        return false;

    ray.tmin = std::max(tmin, tzmin);
    ray.tmax = std::min(tmax, tzmax);
    return true;
}

// This function inserts a color into the framebuffer at position x,y
void Renderer::fillColor(int x, int y, const glm::vec4& color)
{
    const size_t index = static_cast<size_t>(m_config.renderResolution.x * y + x);
    m_frameBuffer[index] = color;
}
}
