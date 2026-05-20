#include "gpu_volume.h"
#include <glm/common.hpp>
#include <iostream>
#include <chrono>

#include <glm/gtx/component_wise.hpp>

namespace volume {

GPUVolume::GPUVolume(const Volume* volume)
    : m_volumeTexture(Texture(volume->getData(), volume->dims()))
    , m_indexTexture(Texture(std::vector<float>(0), glm::ivec3(1)))
    , m_pVolume(volume)
    , m_minMaxValues(std::vector<glm::vec2>())
    , m_brickVolumeSize(glm::ivec3(0))
    , m_brickVolume(std::vector<float>())
    , m_indexVolumeSize(glm::ivec3(0))
    , m_indexVolume(std::vector<glm::vec3>())
    , m_volumeDims(glm::vec3(-1))
    , m_brickSize(-1)
    , m_useBricking(false)
    , m_brickPadding(2)
{
    // The index texture should not interpolate offsets
    m_indexTexture.setInterpolationMode(GL_NEAREST);
}

// ======= DO NOT MODIFY THIS FUNCTION ========
// update the config
// Note: whenever this is called brickSizeChanged(...) should also be called
void GPUVolume::setVolumeConfig(const render::GPUVolumeConfig& config)
{
    m_volumeConfig = config;
}

// ======= DO NOT MODIFY THIS FUNCTION ========
// Full update of the min max data structure and cache neded when loading data or changing brick size
void GPUVolume::brickSizeChanged(render::RenderConfig renderConfig, std::array<float, 256>& opacitySumTable)
{ 
    // only update when the values changed
    if (m_brickSize != m_volumeConfig.brickSize || m_useBricking != m_volumeConfig.useVolumeBricking || m_volumeDims != m_pVolume->dims())
    {
        // set internal brick size, we always assume cubic bricks, i.e., m_brickSize * m_brickSize * m_brickSize
        m_brickSize = m_volumeConfig.brickSize;
        m_useBricking = m_volumeConfig.useVolumeBricking;
        m_volumeDims = m_pVolume->dims();

        updateMinMax();
        updateBrickCache(renderConfig, opacitySumTable);
    }
}

// ======= TODO: IMPLEMENT ========
//
// Part of **3. Volume Bricking**
//
// This function should calculate the minimum and maximum values per brick
// We store the values in the 1D std::vector m_minMaxValues with min and max in a glm::vec2
// This function is very similar to the min max calculation for the blocking you did in Part 2 but with the bricksize
// However, here you need to consider padding and you do not need to store position
//
// The bricksize is set in m_brickSize (as a single integer, as we want cubic bricks)
// To allow interpolation and gradient calculation without recalculating the indirection, we add padding to the bricks
// Use m_brickPadding as the size of the padding, in this implementation it is set to 2 and does not change but be generic
// You need to set m_indexVolumeSize (glm::vec3) as the size (in voxels) of the redirection index
void GPUVolume::updateMinMax()
{
    // we initialize a timer to test this method
    using clock = std::chrono::steady_clock;
    std::chrono::steady_clock::time_point start = clock::now();

    // ======= TODO: calculate m_indexVolumeSize  based on volume dimensions and m_brickSize
    m_indexVolumeSize = glm::vec3(1);

    if (m_volumeConfig.useVolumeBricking) {
        m_indexVolumeSize = (m_volumeDims + glm::ivec3((m_brickSize) - 1)) / (m_brickSize);
    }

    // the number of bricks
    int numBricksLinear = m_indexVolumeSize.x * m_indexVolumeSize.y * m_indexVolumeSize.z;
    // resize m_minMaxValues to take one vec2(min, max) per brick
    m_minMaxValues.resize(numBricksLinear);

    if (!m_volumeConfig.useVolumeBricking) {
        m_minMaxValues[0] = glm::vec2(0.0f, m_pVolume->maximum());
    } else {
        int index = 0;
        for (unsigned int z = 0; z < m_indexVolumeSize.z; z++) {
            for (unsigned int y = 0; y < m_indexVolumeSize.y; y++) {
                for (unsigned int x = 0; x < m_indexVolumeSize.x; x++) {
                    float min = m_pVolume->maximum();
                    float max = 0.0f;

                    for (unsigned int dx = 0; dx < m_brickSize; dx++) {
                        for (unsigned int dy = 0; dy < m_brickSize; dy++) {
                            for (unsigned int dz = 0; dz < m_brickSize; dz++) {

                                const int xVoxel = x * m_brickSize + dx;
                                const int yVoxel = y * m_brickSize + dy;
                                const int zVoxel = z * m_brickSize + dz;

                                if (xVoxel >= m_pVolume->dims().x || yVoxel >= m_pVolume->dims().y || zVoxel >= m_pVolume->dims().z) {
                                    continue;
                                }

                                float value = m_pVolume->getVoxel(xVoxel, yVoxel, zVoxel);
                                min = std::min(min, value);
                                max = std::max(max, value);
                            }
                        }
                    }

                    m_minMaxValues[index] = glm::vec2(min, max);
                    index++;
                }
            }
        }
    }

    // stop the timer
    using clock = std::chrono::steady_clock;
    std::chrono::steady_clock::time_point stop = clock::now();

    std::cout << "updateMinMax() executed in " << std::chrono::duration<double, std::milli>(stop - start).count() << "ms" << std::endl;
}

// ======= TODO: IMPLEMENT ========
//
// Part of **3. Volume Bricking**
//
// This function should calculate whether a brick is active or not
// and adds the offset to the index volume and copies the actual brick data into the cache
// It will be called whenever the volume is loaded, the brick size changes and when the transfer function or iso value changes
// 
// In principle this function is very similar to the updateActiveBlocks function you created for empty space skipping
// However, we also prepare the volume index and cache data for the GPU
// You need to update several variables for this to work downstream
// m_brickVolume: the actual volume cache should contain a brick at positions given by the offsets in the index
// m_brickVolumeSize: the size of the cache volume in 3D in voxels i.e., your result of findOptimalDimensions (num bricks in 3D) times the padded brick size
// m_indexVolume: the index redirection data should contain the offset into m_brickVolume in 3D (consider already normalizing these to save cycles on the GPU)
// NOTE: doing this correctly with padding can be tricky. If you get stuck debugging, consider without padding first.
// NOTE: you should have updated m_indexVolumeSize already in updateMinMax()
void GPUVolume::updateBrickCache(render::RenderConfig renderConfig, std::array<float, 256>& opacitySumTable)
{
    // we initialize a timer to test this method
    using clock = std::chrono::steady_clock;
    std::chrono::steady_clock::time_point start = clock::now();

    // no bricking means we just add a single item and the cache becomes the volume
    if (!m_volumeConfig.useVolumeBricking) { // if volume bricking is turned off just return the entire volume
        m_volumeTexture.update(m_pVolume->getData(), m_pVolume->dims());
        m_indexTexture.update(std::vector<glm::vec4> { glm::vec4(0) }, glm::ivec3(1));
        return;
    }


    // NOTE: m_indexVolume needs to be filled in the right order to work as a 3D volume on the GPU
    // (0,0,0) should be in index 0, (1,0,0) = 1, etc.
    // m_brickVolume can be filled in any way, as long as you keep correct offsets in your index
    // as long as you implement the redirected lookup on the GPU in the same way
    // we update the textures with the calculated values
    // NOTE: this might crash as long as m_brickVolumeSize and m_indexVolumeSize are not set correctly when enabling bricking

    updateMinMax();

    int numBlocks = m_indexVolumeSize.x * m_indexVolumeSize.y * m_indexVolumeSize.z;

    std::vector<glm::ivec4> activeBlocks;
    std::vector<unsigned int> isActiveAndIndex;

    int currentBlock = 0;
    int activeBlockCount = 0;
    for (unsigned int z = 0; z < m_indexVolumeSize.z; z++) {
        for (unsigned int y = 0; y < m_indexVolumeSize.y; y++) {
            for (unsigned int x = 0; x < m_indexVolumeSize.x; x++) {
                glm::vec2 minMax = m_minMaxValues[currentBlock];
                float diff = minMax.y - minMax.x;

                if (renderConfig.renderMode == render::RenderMode::RenderComposite) {
                    diff = opacitySumTable[static_cast<int>(minMax.y)] - opacitySumTable[static_cast<int>(minMax.x)];
                }

                if (diff > 1.0f) {
                    // active
                    isActiveAndIndex.push_back(activeBlockCount);
                    activeBlocks.push_back(glm::ivec4(x, y, z, currentBlock));
                } else {
                    isActiveAndIndex.push_back(-1);
                }

                currentBlock++;
            }
        }
    }

    this->m_indexVolumeSize = (m_pVolume->dims() + (m_brickSize - 1)) / m_brickSize;
    this->m_trueIndexVolumeSize = glm::vec3(m_pVolume->dims()) / (float)m_brickSize;
    
    glm::ivec3 optimalDimensions = findOptimalDimensions(activeBlocks.size());
    glm::ivec3 bufferVolumeSizeBrick = optimalDimensions * (m_brickSize + m_brickPadding);

    m_indexVolume.clear();
    // indexVolume will contain for each brick the offset in the brickVolume
    // if the brick is not active, the offset will be -1

    for (unsigned int j = 0; j < numBlocks; j++) {
        m_indexVolume.push_back(glm::vec3(-1));
    }

    currentBlock = 0;
    for (unsigned int z = 0; z < optimalDimensions.z; z++) {
        for (unsigned int y = 0; y < optimalDimensions.y; y++) {
            for (unsigned int x = 0; x < optimalDimensions.x; x++) {

                while (true) {

                    if (currentBlock >= numBlocks) {
                        break;
                    }

                    // here we want to find the offset in the buffer volume
                    int activeBlock = isActiveAndIndex[currentBlock];

                    if (activeBlock == -1) {
                        currentBlock++;
                        continue;
                    }

                    glm::ivec3 block = glm::ivec3(x, y, z) * (m_brickSize + m_brickPadding);

                    m_indexVolume[currentBlock] = block;
                    break;
                }

                currentBlock++;
            }
        }
    }

    


    std::vector<std::vector<std::vector<float>>> volumeData = std::vector<std::vector<std::vector<float>>>(bufferVolumeSizeBrick.z, std::vector<std::vector<float>>(bufferVolumeSizeBrick.y, std::vector<float>(bufferVolumeSizeBrick.x, 0.0f)));

    for (unsigned int i = 0; i < activeBlocks.size(); i++) {
        glm::ivec3 indexInOriginalData = glm::vec3(activeBlocks[i]); // block index in 3D in the OLD data structure
        unsigned int blockIndex = activeBlocks[i].a; // block index in 1D

        glm::ivec3 block = m_indexVolume[blockIndex]; // block index in 3D in the NEW data structure

        glm::vec2 minMax = m_minMaxValues[blockIndex];

        for (unsigned int z = 0; z < m_brickSize + m_brickPadding; z++) {
            for (unsigned int y = 0; y < m_brickSize + m_brickPadding; y++) {
                for (unsigned int x = 0; x < m_brickSize + m_brickPadding; x++) {
                    glm::ivec3 oldDataLocation = indexInOriginalData * (m_brickSize) + glm::ivec3(x, y, z);
                    glm::ivec3 newDataLocation = block + glm::ivec3(x, y, z);

                    if (newDataLocation.x >= bufferVolumeSizeBrick.x || newDataLocation.y >= bufferVolumeSizeBrick.y || newDataLocation.z >= bufferVolumeSizeBrick.z) {
                        continue;
                    }

                    if (oldDataLocation.x >= m_pVolume->dims().x || oldDataLocation.y >= m_pVolume->dims().y || oldDataLocation.z >= m_pVolume->dims().z) {
                        volumeData[newDataLocation.z][newDataLocation.y][newDataLocation.x] = 0.0f;
                    }
                    else {

                        volumeData[newDataLocation.z][newDataLocation.y][newDataLocation.x] = m_pVolume->getVoxel(oldDataLocation.x, oldDataLocation.y, oldDataLocation.z);
                    }
                }
            }
        }
    }

    m_brickVolumeSize = bufferVolumeSizeBrick;

    m_brickVolume.clear();
    for (unsigned int z = 0; z < bufferVolumeSizeBrick.z; z++) {
        for (unsigned int y = 0; y < bufferVolumeSizeBrick.y; y++) {
            for (unsigned int x = 0; x < bufferVolumeSizeBrick.x; x++) {
                m_brickVolume.push_back(volumeData[z][y][x]);
            }
        }
    }

    m_volumeTexture.update(m_brickVolume, m_brickVolumeSize);
    m_indexTexture.update(m_indexVolume, m_indexVolumeSize);

    // stop the timer
    using clock = std::chrono::steady_clock;
    std::chrono::steady_clock::time_point stop = clock::now();

    std::cout << "updateCache() executed in " << std::chrono::duration<double, std::milli>(stop - start).count() << "ms" << std::endl;
}

// ======= TODO: IMPLEMENT ========
//
// Part of **3. Volume Bricking**
//
// This helper function should calculate the size of the cache volume in number of bricks in 3D
// The input N is the total number of bricks
// The input maxSize is the maximum texture size divided by the (padded) brick size
// The output is the number of bricks when tightly packing it into a 3D structure
// This is needed as the extent of textures on the GPU is limited
// There are various ways to do this, the goal is to find a 3D volume that has little empty space
// One that always works is taking the smallest cube that fits N, i.e., the next highest integer of the cubic root of N
// However this can be very wasteful
// E.g., N=10 bricks could be packed into a 5x2x1 cube without waste or the next cubic root 4x4x4 cuboid with six blocks of wasted space
// The goal of this function is to find a cuboid with little wasted space that fits into the texture extents in a short time
glm::ivec3 GPUVolume::findOptimalDimensions(int N)
{
    // we initialize a timer to test this method
    using clock = std::chrono::steady_clock;
    std::chrono::steady_clock::time_point start = clock::now();

    int maxSize;
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &maxSize);

    // TODO: calculate the optimal dimensions for the cache volume here
    glm::ivec3 optimalDimensions = glm::vec3(1, 1, 1);

    int lowestCost { N * N * N };

    int maxAlongOneAxis = glm::floor(maxSize / (m_brickSize + m_brickPadding));
    std::cout << "maxAlongOneAxis: " << maxAlongOneAxis << std::endl;

    if (N <= maxAlongOneAxis) {
        optimalDimensions = glm::ivec3(N, 1, 1);
    }
    else {
        bool found = false;
        for (unsigned int z = 0; z < maxAlongOneAxis; z++) {
            if (found)
                break;
            for (unsigned int y = 0; y < maxAlongOneAxis; y++) {
                if (found)
                    break;
                for (unsigned int x = 0; x < maxAlongOneAxis; x++) {
                    if (x * y * z > N) {
                        int cost = x * y * z;
                        if (cost < lowestCost) {
                            lowestCost = cost;
                            optimalDimensions = glm::ivec3(x, y, z);
                        }
                    } else if (x * y * z == N) {
                        optimalDimensions = glm::ivec3(x, y, z);
                        found = true;
                        break;
                    }
                }
            }
        }
    }

    // stop the timer
    using clock = std::chrono::steady_clock;
    std::chrono::steady_clock::time_point stop = clock::now();

    std::cout << "findOptimalDimensions() with cube executed in " << std::chrono::duration<double, std::milli>(stop - start).count() << "ms" << std::endl;
    std::cout << "fitting cube uses " << (float)N/(optimalDimensions.x*optimalDimensions.y*optimalDimensions.z)*100.0f << "\% of available space" << std::endl;
    std::cout << "optimal dimensions: " << optimalDimensions.x << "x" << optimalDimensions.y << "x" << optimalDimensions.z << " for N=" << N << std::endl;

    // return the optimal dimensions
    return optimalDimensions;
}

// ======= DO NOT MODIFY THIS FUNCTION ========
// get the OpenGL id of the volume / cache texture
GLuint GPUVolume::getTexId() const 
{
    return m_volumeTexture.getTexId();
}

// ======= DO NOT MODIFY THIS FUNCTION ========
// get the OpenGL id of the index texture
GLuint GPUVolume::getIndexTexId() const
{
    return m_indexTexture.getTexId();
}

// ======= DO NOT MODIFY THIS FUNCTION ========
// on the GPU to switch between nearest and linear we just have to update the texture properties
void GPUVolume::updateInterpolation()
{
    if (interpolationMode == InterpolationMode::NearestNeighbour) {
        m_volumeTexture.setInterpolationMode(GL_NEAREST);
    } else {
        m_volumeTexture.setInterpolationMode(GL_LINEAR);
    }
}
}