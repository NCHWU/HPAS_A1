#pragma once

#include <queue>
#include <iostream>
#include <glm/glm.hpp>
#include "volume/volume.h"
#include "render/render_config.h"
#include <limits>
#include <algorithm>

enum class Occupancy {
    Empty = 0,
    NonEmpty = 1,
};

struct BoundingBoxGeometry {
    glm::vec3 min;
    int occupancyClass;
    glm::vec3 max;
    int isFront;
};

class OHTNode {
public:
    glm::ivec3 boundingBoxMin;
    glm::ivec3 boundingBoxMax;
    bool isLeaf;
    Occupancy occupancy; // Only used if it's a leaf node
    OHTNode* children[8]; // Only used if it's an internal node
    glm::ivec2 occupancyHistogram; // First is empty, second is non-empty
    OHTNode* parent;

    OHTNode(glm::ivec3 boundingBoxMin, glm::ivec3 boundingBoxMax, bool isLeaf, OHTNode* parent = nullptr, Occupancy occupancy = Occupancy::Empty)
        : boundingBoxMin(boundingBoxMin)
        , boundingBoxMax(boundingBoxMax)
        , isLeaf(isLeaf)
        , occupancy(occupancy)
        , parent(parent)
    {
        if (!isLeaf) {
            for (int i = 0; i < 8; i++) {
                children[i] = nullptr;
            }
        }

        occupancyHistogram = glm::ivec2(0);

        if (isLeaf) {
            if (occupancy == Occupancy::Empty) {
                occupancyHistogram.x = 1;
            } else {
                occupancyHistogram.y = 1;
            }
        }
    }

    Occupancy getOccupancyClass() const
    {
        // If it's a leaf node, return the occupancy class directly
        if (isLeaf) {
            return occupancy;
        }

        // Majority voting for internal nodes
        if (occupancyHistogram.x > occupancyHistogram.y) {
            return Occupancy::Empty;
        } else {
            return Occupancy::NonEmpty;
        }
    }

    void print(int depth = 0)
    {
        for (int i = 0; i < depth; i++) {
            std::cout << "  ";
        }

        std::cout << "Node: " 
            << boundingBoxMin.x << ", " << boundingBoxMin.y << ", " << boundingBoxMin.z 
            << " - " 
            << boundingBoxMax.x << ", " << boundingBoxMax.y << ", " << boundingBoxMax.z 
            << ", hist: " << occupancyHistogram.x << ", " << occupancyHistogram.y
            << ", class: " << (getOccupancyClass() == Occupancy::Empty ? "Empty" : "NonEmpty")
            <<  std::endl;

        if (!isLeaf) {
            for (int i = 0; i < 8; i++) {
                children[i]->print(depth + 1);
            }
        }
    }

    /*
     Recursively sort the children of this node based on their distance to the camera.
     The children are sorted in descending order, so the farthest child is first.
     This is done to ensure that the farthest child is processed first in the ray casting algorithm.
     * cameraPos: The position of the camera in world coordinates
    **/
    void sort(const glm::vec3 cameraPos)
    {
        // Leafs need not be sorted
        if (this->isLeaf) {
            return;
        }

        std::sort(&this->children[0], &this->children[8], [cameraPos](const OHTNode* a, const OHTNode* b) {
            // ======= TODO: IMPLEMENT ========
            // Sort the children based on their distance to the camera
            // Sort in descending order, so the farthest child is first

            // Calculate the center of the bounding box for both children
            glm::vec3 centerA = (glm::vec3(a->boundingBoxMin) + glm::vec3(a->boundingBoxMax)) * 0.5f;
            glm::vec3 centerB = (glm::vec3(b->boundingBoxMin) + glm::vec3(b->boundingBoxMax)) * 0.5f;

            // Calculate the squared distance from the camera to the center of each child
            float distanceA = glm::dot(centerA - cameraPos, centerA - cameraPos);
            float distanceB = glm::dot(centerB - cameraPos, centerB - cameraPos);

            // Return true if child a is farther from the camera than child b, so that a comes before b in the sorted order
            return distanceA > distanceB;

        });

        // Sort the children of this node based on their distance to the camera
        for (int i = 0; i < 8; i++) {
            if (children[i] != nullptr) {
                children[i]->sort(cameraPos);
            }
        }
    }


    /* 
     * Produce a list of bounding box geometry instances, which define everything we need to
     * know about a bounding box to draw it.
     * Also produces a list of indices corresponding to geometry instances.
     * Each geometry instance holds an index into this list, which holds the index of the first child.
     * The child count is also stored in the geometry instance.
    **/
    void traverse(std::vector<BoundingBoxGeometry>& emittedGeometry)
    {
        OHTNode* node = this;
        OHTNode* parent = node->parent;

        bool addedGeometry = false;
        BoundingBoxGeometry geometry;

        // ======= TODO: IMPLEMENT ========
        // Emit a (back-facing) bounding box for this node if it is either:
        // 1) the root node
        // 2) any internal or leaf node that has a different occupancy class than its parent
        // Then, recursively traverse the children of this node and emit their bounding boxes as well.
        // Finally, also emit the front-facing bounding box for this node.

        bool shouldEmit = false;

        if (parent == nullptr) {
            shouldEmit = true;
        } else if (node->getOccupancyClass() != parent->getOccupancyClass()) {
            shouldEmit = true;
        }

        if (shouldEmit) {
            // Emit back-facing bounding box
            geometry.min = glm::vec3(node->boundingBoxMin);
            geometry.max = glm::vec3(node->boundingBoxMax);
            geometry.occupancyClass = static_cast<int>(node->getOccupancyClass());
            geometry.isFront = 0;
            emittedGeometry.push_back(geometry);
            addedGeometry = true;
        }

        if (!node->isLeaf) {

            for (int i = 0; i < 8; i++) {
                if (node->children[i] != nullptr) {
                    node->children[i]->traverse(emittedGeometry);
                }
            }
        }

        if (addedGeometry) {
            geometry.isFront = 1;
            emittedGeometry.push_back(geometry);
        }
      
    }

    /*
     Build an OHTNode tree from the full volume using a transfer function and render mode
     * volume: Pointer to the input volume
     * tfColorMap: Transfer function colormap
     * renderMode: Render mode
    **/
    static OHTNode* build(const volume::Volume* volume, render::RenderConfig& renderConfig)
    {
        return build(volume, glm::ivec3(0), volume->dims(), renderConfig);
    }

    /*
     Recursively build the OHTNode tree based on subregion of the volume
     * volume: Pointer to the input volume
     * min: Minimum corner of the subregion
     * max: Maximum corner of the subregion
     * tfColorMap: Transfer function colormap
     * renderMode: Render mode
    **/
    static OHTNode* build(const volume::Volume* volume, glm::ivec3 min, glm::ivec3 max, render::RenderConfig& renderConfig)
    {
        // Stop condition
        int minBoxSize = 16;
        if (max.x - min.x <= minBoxSize || max.y - min.y <= minBoxSize || max.z - min.z <= minBoxSize) {
            // Spawn a leaf node

            // ======= TODO: IMPLEMENT ========
            // Spawn a leaf node, making sure that the occupancy class is set correctly
            // The parent reference can be set to nullptr, since it is assigned later

            // Get isovalue, transfer function size, start and range for the transfer function from the render config
            float isovalue = renderConfig.isoValue;
            int tfSize = renderConfig.tfColorMap.size();
            float start = renderConfig.tfColorMapIndexStart;
            float range = renderConfig.tfColorMapIndexRange;

            bool isNonEmpty = false;

            // Calculate the min and max values for the block
            float minVal = std::numeric_limits<float>::max();
            float maxVal = std::numeric_limits<float>::lowest();

            // Iterate through the voxels in the block to find the min and max values, and check if the block is non-empty based on the render mode
            for (int z = min.z; z < max.z && !isNonEmpty; ++z) {
                for (int y = min.y; y < max.y && !isNonEmpty; ++y) {
                    for (int x = min.x; x < max.x; ++x) {

                        // Get the voxel value at (x, y, z)
                        float voxelValue = volume->getVoxel(x, y, z);

                        // Update the min and max values for the block
                        minVal = std::min(minVal, voxelValue);
                        maxVal = std::max(maxVal, voxelValue);

                        // For compositing, a block is non-empty if there is any opacity in the transfer function between the min and max values of the block
                        if (renderConfig.renderMode == render::RenderMode::RenderComposite) {

                            // If the range is 0, it means all values in the block are the same, so we only need to check one value
                            if (range > 0.0f) {

                                // For compositing, we need to check if there is any opacity in the transfer function for the range of values in the block
                                int index = static_cast<int>((voxelValue - start) / range * tfSize);
                                index = std::clamp(index, 0, tfSize - 1);

                                // Get the alpha value from the transfer function color map
                                float alpha = renderConfig.tfColorMap[index].a;

                                // If there is any opacity, the block is non-empty
                                if (alpha > 0.01f) { // small threshold
                                    isNonEmpty = true;
                                    break;
                                } else {
                                    isNonEmpty = true; // safe fallback
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            // For isosurface rendering, a block is active if the iso value is between the min and max values of the block
            if (renderConfig.renderMode == render::RenderMode::RenderIso) {

                // For isosurface rendering, a block is active if the iso value is between the min and max values of the block
                isNonEmpty = (isovalue >= minVal && isovalue <= maxVal) ? true : false;
            }
                    
            // Create the leaf node with the correct occupancy class
            return new OHTNode(min, max, true, nullptr, isNonEmpty ? Occupancy::NonEmpty : Occupancy::Empty);
        }

        // Create the internal node
        OHTNode* node = new OHTNode(min, max, false);

        // Continue recursion
        glm::ivec3 mid = (min + max) / 2;

        node->children[0] = build(volume, min, mid, renderConfig);
        node->children[1] = build(volume, glm::ivec3(mid.x, min.y, min.z), glm::ivec3(max.x, mid.y, mid.z), renderConfig);
        node->children[2] = build(volume, glm::ivec3(min.x, mid.y, min.z), glm::ivec3(mid.x, max.y, mid.z), renderConfig);
        node->children[3] = build(volume, glm::ivec3(mid.x, mid.y, min.z), glm::ivec3(max.x, max.y, mid.z), renderConfig);
        node->children[4] = build(volume, glm::ivec3(min.x, min.y, mid.z), glm::ivec3(mid.x, mid.y, max.z), renderConfig);
        node->children[5] = build(volume, glm::ivec3(mid.x, min.y, mid.z), glm::ivec3(max.x, mid.y, max.z), renderConfig);
        node->children[6] = build(volume, glm::ivec3(min.x, mid.y, mid.z), glm::ivec3(mid.x, max.y, max.z), renderConfig);
        node->children[7] = build(volume, mid, max, renderConfig);

        // Set all parent references
        for (int i = 0; i < 8; i++) {
            node->children[i]->parent = node;
        }

        // Fill in the occupancy histogram
        for (int i = 0; i < 8; i++) {
            node->occupancyHistogram += node->children[i]->occupancyHistogram;
        }

        return node;
    }

    // Destructor
    ~OHTNode()
    {
        if (!isLeaf) {
            for (int i = 0; i < 8; i++) {
                delete children[i];
            }
        }
    }
};
