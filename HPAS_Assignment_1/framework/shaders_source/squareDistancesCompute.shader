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
uniform int rowsIterationOffset;

// Function to compare two ValueAndIndex for sorting
int compare(const ValueAndIndex a, const ValueAndIndex b) {
    return a.value < b.value ? -1 : (a.value > b.value ? 1 : 0);
}

void main() {
    uint i = gl_GlobalInvocationID.x; // Row being processed

    int row = rowsIterationOffset + int(i);
    int allDistancesOffset = int(i) * numRows;
    
    for (int j = 0; j < numNeighbours; j++)
    {
        ValueAndIndex lowestDistance = allDistances[allDistancesOffset + 0];
        int lowestIndex = 0;

        // Finding the current lowest distance
        for (int k = 1; k < numRows; k++) {
            if (allDistances[allDistancesOffset + k].value <= lowestDistance.value) {
                lowestDistance = allDistances[allDistancesOffset + k];
                lowestIndex = k;
            }
        }

        // Selecting the point with the lowest distance
        sqDistances[row * numNeighbours + j] = lowestDistance;
        //sqDistances[row * numNeighbours + j] = ValueAndIndex(2.0, 1);

        // Setting the lowest distance to a high value to avoid selecting it again
        allDistances[allDistancesOffset + lowestIndex].value = 10000000.;
    }
}
