#version 460 core

layout(local_size_x = 8) in;

layout(std430, binding = 0) readonly buffer DataBuffer {
    float data[];
};

struct ValueAndIndex {
	float value;
	int index;
};

layout(std430, binding = 1) writeonly buffer SqDistanceBuffer {
    ValueAndIndex sqDistances[];
};

layout(std430, binding = 2) buffer AllDistanceBuffer {
    ValueAndIndex allDistances[];
};



uniform int numRows;
uniform int numCols;
uniform int numNeighbours;
uniform int rowsPerIteration;
uniform int rowsIterationOffset;
uniform int rowsPerIterationInner;
uniform int rowsIterationOffsetInner;

void main()
{
    uint i = gl_GlobalInvocationID.x;

    int row = rowsIterationOffset + int(i);

    // Start-end is the batch of points currently being calculated
    int start = rowsIterationOffsetInner;
    int end = min(start + rowsPerIterationInner, numRows);

    // Calculate distances
    for (int j = start; j < end; j++)
    {
        // Ignoring distance to self
        if (row == j) {
            allDistances[i * numRows + j] = ValueAndIndex(10000000., j); // Ignore self
            continue;
        }

        float sqDistance = 0.0f;
        // Calculate squared distance to the other point
        for (int k = 0; k < numCols; k++) {
            float diff = data[row * numCols + k] - data[j * numCols + k];
            sqDistance += diff * diff;
        }
        allDistances[i * numRows + j] = ValueAndIndex(sqDistance, j);
    }
}
