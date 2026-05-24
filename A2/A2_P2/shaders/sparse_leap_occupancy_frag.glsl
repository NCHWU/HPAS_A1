#version 410
#extension GL_ARB_shader_storage_buffer_object : enable
#extension GL_ARB_shader_image_load_store : enable
#extension GL_ARB_shader_atomic_counters : enable

uniform int SCREEN_WIDTH;
uniform vec2 screenSize;
uniform float fov;
uniform mat4 viewMatrix;

struct RayEvent {
    float distanceFromCamera;
    int isEntry;
    int occupancyClass;
    uint next;
};

struct BoundingBox {
    vec3 minCoord;
    int occupancyClass;
    vec3 maxCoord;
    int isFront;
};

layout(std430, binding = 0) buffer HeadBuffer {
    uint heads[];
};

layout(std430, binding = 1) buffer RayEventBuffer {
    RayEvent rayEvents[];
};

layout(std430, binding = 3) buffer BoundingBoxBuffer {
    BoundingBox boundingBoxes[];
};

layout(binding = 2, offset = 0) uniform atomic_uint nodeCounter; // Atomic counter for node allocation

out vec4 FragColor;
uniform vec3 cameraPos;

// Defintions of functions that do not need to be changed
// For reference, see implementations below main
vec3 getRayDirection(vec2 pixelPosIndex);
bool intersectBoundingBox(BoundingBox box, vec3 rayOrigin, vec3 rayDir, out float tMin, out float tMax);
void insertRayEvent(float distanceFromCamera, int isEntry, int occupancyClass, inout uint head);

void main() {
    uint pixelIndex = uint(gl_FragCoord.y) * SCREEN_WIDTH + uint(gl_FragCoord.x); // Compute 1D pixel index
    uint head = 0;

    // The direction of this pixel's ray from the camera position
    vec3 rayDir = getRayDirection(gl_FragCoord.xy);

    FragColor = vec4(rayDir, 1.0); // Default color

    // Sum of values for visualisation purposes
    vec3 sum = vec3(0.);

    int numBoundingBoxes = boundingBoxes.length();
	float tMin, tMax;
    
    // ======= TODO: IMPLEMENT ========
    // Iterate through all bounding boxes
    // and check if the ray intersects with each box.
    // Then use that information to create ray events.

    //Check each bounding box for intersection with the ray
    for (int i = 0; i < numBoundingBoxes; i++) {

        // Get the current bounding box
        BoundingBox box = boundingBoxes[i];

        // If the ray intersects with the bounding box, create ray events for entry and exit points
        if (intersectBoundingBox(box, cameraPos, rayDir, tMin, tMax)) {

            // Insert ray events for entry and exit points
            if (box.isFront == 1) {
                insertRayEvent(max(tMin, 0.0), 1, box.occupancyClass, head);
            } else {
                insertRayEvent(tMax, 0, box.occupancyClass, head);
            }

            // For visualization: accumulate occupancy class values along the ray
            //sum += vec3(box.occupancyClass) * 0.05;


            // Skip colouring the root back/front boxes in debug view only.
            // Root is usually first and last because traverse emits root before children and after children.

           // Strong debug visualization: show every intersected box clearly
            vec3 debugColor;

            if (box.occupancyClass == 1) {
                debugColor = vec3(1.0, 0.25, 0.0);   // non-empty = orange/red
            } else {
                debugColor = vec3(0.0, 0.6, 1.0);    // empty = bright cyan/blue
            }

            if (box.isFront == 1) {
                debugColor = mix(debugColor, vec3(1.0), 0.25); // make front face lighter
            }

            sum += debugColor * 0.12;

        }
    }

    // Visualize the accumulated sum
   // FragColor = vec4(sum, 1.0);
   FragColor = vec4(clamp(sum, 0.0, 1.0), 1.0);

    // Store the head of the linked list in the heads array
    heads[pixelIndex] = head;
}


// ======= DO NOT MODIFY THIS FUNCTION ========
// get the ray direction from the pixel position
vec3 getRayDirection(vec2 pixelPosIndex)
{
    vec3 dir;
    
    float d = 1 / tan(radians(fov) / 2);
    float aspect_ratio = screenSize.x / screenSize.y;

    dir.x = aspect_ratio * (2 * pixelPosIndex.x / screenSize.x) - aspect_ratio;
    dir.y = (2 * pixelPosIndex.y / screenSize.y) - 1.;
    dir.z = d;
    dir = normalize(dir);

    //dir.x = -dir.x;
    dir.z = -dir.z;

    dir = (viewMatrix * vec4(dir, 0.0)).xyz;
    dir = normalize(dir);

    return dir;
}

/* ======= DO NOT MODIFY THIS FUNCTION ========
 Calculate the intersection of a ray with a bounding box
 Will return false if the ray does not intersect the box
 Otherwise it will fill the tMin and tMax values
 * box: the bounding box to check against
 * rayOrigin: the origin of the ray
 * rayDir: the direction of the ray
 * tMin (only output): the distance from the ray origin to the first intersection point
 * tMax (only output): the distance from the ray origin to the second intersection point
**/
bool intersectBoundingBox(BoundingBox box, vec3 rayOrigin, vec3 rayDir, out float tMin, out float tMax) {
    vec3 invDir = 1.0 / rayDir;
    vec3 t0s = (box.minCoord - rayOrigin) * invDir;
    vec3 t1s = (box.maxCoord - rayOrigin) * invDir;
    
    vec3 tMinVec = min(t0s, t1s);
    vec3 tMaxVec = max(t0s, t1s);
    
    tMin = max(tMinVec.x, max(tMinVec.y, tMinVec.z));
    tMax = min(tMaxVec.x, min(tMaxVec.y, tMaxVec.z));
    
    return tMax >= max(tMin, 0.0); // Ensures the intersection is in front of the ray origin
}

/* ======= DO NOT MODIFY THIS FUNCTION ========
 Insert a new ray event into front of the linked list.
 * distanceFromCamera: the distance from the camera to the ray event
 * isEntry: 1 if the ray event is an entry, 0 if it is an exit
 * occupancyClass: the occupancy class of the ray event
 * head: the head of the linked list (will be updated)
**/
void insertRayEvent(float distanceFromCamera, int isEntry, int occupancyClass, inout uint head) {
    uint newNodeIndex = atomicCounterIncrement(nodeCounter); // Atomically allocate a node

    // Store data in the new node
    rayEvents[newNodeIndex].distanceFromCamera = distanceFromCamera;
    rayEvents[newNodeIndex].isEntry = isEntry;
    rayEvents[newNodeIndex].occupancyClass = occupancyClass;

    // Insert node at the head of the list
    rayEvents[newNodeIndex].next = head;
    head = newNodeIndex;
}
