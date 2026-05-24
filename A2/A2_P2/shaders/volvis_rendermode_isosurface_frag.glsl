#version 410
#extension GL_ARB_shader_storage_buffer_object : enable
#extension GL_ARB_shader_atomic_counters : enable

uniform int SCREEN_WIDTH;
uniform vec2 screenSize;
uniform float fov;
uniform mat4 viewMatrix;
uniform vec3 cameraPos;
uniform float stepSize;
uniform bool doCountSamples;

uniform int doEmptySpaceSkipping;

#define EVENT_OCCUPANCY_EMPTY 0
#define EVENT_OCCUPANCY_NONEMPTY 1

struct RayEvent {
    float distanceFromCamera;
    int isEntry;
    int occupancyClass;
    uint next;
};

layout(std430, binding = 0) buffer HeadBuffer {
    uint heads[];
};

layout(std430, binding = 1) buffer RayEventBuffer {
    RayEvent rayEvents[];
};

layout(binding = 4, offset = 0) uniform atomic_uint sampleCounter; // Atomic counter for sample count

out vec4 FragColor;

in vec2 TexCoords;

// the front faces texture
uniform sampler2D frontFaces;

// the back faces texture
uniform sampler2D backFaces;

// the volume or volume cache when using indirection
uniform sampler3D volumeData;

// the volume indirection lookup
uniform sampler3D volumeIndexData;

// this contains the voxels size in normalized coordinates + 0 if using regular texture and 1 when using bricking
uniform vec4 volumeInfo; // (voxelsize.x, voxelsize.y, voxelsize.z, use bricking?)


// contains various rendering options, here stepsize and its reciprocal, the isoValue and the toggle to use shading
// Note: we give reciprocals to avoid divisions as they are more expensive than multiplications
// so if we can calculate a reciprocal once outside instead of potentially multiple times every thread it can save a lot of computation
uniform vec4 renderOptions; // (stepSize, 1.0f / stepSize, isoValue, useShading)

// Phong shading constants, defined globally to avoid repeated creation in phongShading function
const float ambientCoefficient = 0.1;
const float diffuseCoefficient = 0.6;
const float specularCoefficient = 0.5;
const int specularPower = 32;

// ======= DO NOT MODIFY THIS FUNCTION ========
// calculate Phong shading the same way as in the CPU renderer
// we give the sample color and return the shaded color
vec3 phongShading(vec3 color, vec4 normal, vec3 L, vec3 V)
{
    // If the gradient magnitude is zero, return black
    if (normal.a == 0.0) {
        return vec3(0.0);
    }

    // Ensure the normal is always oriented towards the viewer
    vec3 N = normalize(normal.xyz);
    if (dot(N, V) < 0.0) {
        N = -N;
    }

    // Phong reflection model (assuming white light, thus no contribution to the color)
    vec3 ambient = ambientCoefficient * color;
    vec3 diffuse = diffuseCoefficient * clamp(dot(L, N), 0.0, 1.0) * color;
    vec3 R = 2.0 * dot(L, N) * N - L;
    vec3 specular = specularCoefficient * pow(clamp(dot(R, V), 0.0, 1.0), specularPower) * color;

    return ambient + diffuse + specular;
}

// Sample the data at this position
// The position is in voxel coordinates
float sampleData(vec3 at) {
    at = at * volumeInfo.xyz;

    // Check out of bounds
    if (at.x < 0.0 || at.x > 1.0 || at.y < 0.0 || at.y > 1.0 || at.z < 0.0 || at.z > 1.0) {
		return 0.0;
	}

	return texture(volumeData, at).r;
}

// ======= TODO: IMPLEMENT ========
// This function should calculate the gradient on the fly at the current samplePos
// Gives the voxelSize (in normalized volume coordinates) for the offset
// The return is a vec4 intended to contain the gradient magnitude in the fourth component though not strictly necessary
// Note: The function can be copied over to compositing shader once implemented
vec4 calculateGradient(vec3 samplePos, vec3 voxelSize)
{
    vec4 gradient = vec4(0.0);

    vec3 dx = vec3(1.0, 0.0, 0.0);
    vec3 dy = vec3(0.0, 1.0, 0.0);
    vec3 dz = vec3(0.0, 0.0, 1.0);

    vec3 maxVoxel = (1.0 / volumeInfo.xyz) - vec3(1.0);

    vec3 px1 = clamp(samplePos + dx, vec3(0.0), maxVoxel);
    vec3 px0 = clamp(samplePos - dx, vec3(0.0), maxVoxel);

    vec3 py1 = clamp(samplePos + dy, vec3(0.0), maxVoxel);
    vec3 py0 = clamp(samplePos - dy, vec3(0.0), maxVoxel);

    vec3 pz1 = clamp(samplePos + dz, vec3(0.0), maxVoxel);
    vec3 pz0 = clamp(samplePos - dz, vec3(0.0), maxVoxel);

    float fx1 = sampleData(px1);
    float fx0 = sampleData(px0);

    float fy1 = sampleData(py1);
    float fy0 = sampleData(py0);

    float fz1 = sampleData(pz1);
    float fz0 = sampleData(pz0);

    gradient.xyz = vec3(fx1 - fx0, fy1 - fy0, fz1 - fz0);
    gradient.a = length(gradient.xyz);

    return gradient;
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

    dir.z = -dir.z;

    dir = (viewMatrix * vec4(dir, 0.0)).xyz;
    dir = normalize(dir);

    return dir;
}

/*
 Sample the volume data along the ray direction
 * startT: the start of the ray segment (distance along the ray from the camera)
 * endT: the end of the ray segment (distance along the ray from the camera)
 * direction: the ray direction
 * cameraPos: the camera position
 * C: the accumulated color
 * A: the accumulated alpha
**/
bool sampleRange(float startT, float endT, vec3 direction, vec3 cameraPos, out vec3 color) {

    float isoValue = renderOptions.z;
    vec3 surfaceColor = vec3(1,1,0);

    float previousT = startT;
    vec3 previousPos = cameraPos + previousT * direction;
    float previousValue = sampleData(previousPos);


    for(float t = startT + stepSize; t <= endT; t += stepSize){
        if (doCountSamples) {
            atomicCounterIncrement(sampleCounter);
        }

        // ====== TODO: IMPLEMENT ========
        // Sample the volume data and check if it is above the isoValue.
        // Then compute the gradient and use it to shade the surface

        vec3 samplePos = cameraPos + t * direction;
         float currentValue = sampleData(samplePos);

        bool surface_crossed = (previousValue < isoValue && currentValue >= isoValue) || (previousValue > isoValue && currentValue <= isoValue);

        if (surface_crossed) {

            // Calculate the intersection point using linear interpolation
            float denom = currentValue - previousValue;
            float factor = 0.0;

            if (abs(denom) > 1e-6) {
                factor = (isoValue - previousValue) / denom;
            }

            float hitT = mix(previousT, t, clamp(factor, 0.0, 1.0));
            vec3 hitPos = cameraPos + hitT * direction;

            // Calculate the gradient at the intersection point
            vec4 gradient = calculateGradient(hitPos, volumeInfo.xyz);

            // If shading is enabled, calculate the color using Phong shading, otherwise use the surface color directly
           if (renderOptions.w > 0.5) {
                vec3 V = normalize(cameraPos - hitPos);
                vec3 L = V;

                // Calculate the color using Phong shading
                color = phongShading(surfaceColor, gradient, L, V);
            } else {

            // No shading, use the surface color directly
                color = surfaceColor;
            }

            return true;
        }

        // Update previous values for the next iteration
        previousT = t;
        previousValue = currentValue;

    }
    return false;
}

void main()
{
    uint pixelIndex = uint(gl_FragCoord.y) * SCREEN_WIDTH + uint(gl_FragCoord.x);
    uint currentEventPointer = heads[pixelIndex];
    
    vec3 direction = getRayDirection(gl_FragCoord.xy);
    
    // If empty space skipping is disabled, simply use the start and end positions
    // to sample all along the volume data (naive approach)
    if (doEmptySpaceSkipping == 0) {
        vec3 startPos = texture(frontFaces, TexCoords).xyz / volumeInfo.xyz;
        vec3 endPos = texture(backFaces, TexCoords).xyz/ volumeInfo.xyz;

        float startT = length(startPos - cameraPos);
        float endT = length(endPos - cameraPos);
        
        vec3 color;
        bool hit = sampleRange(startT, endT, direction, cameraPos, color);

        if (hit) {
            FragColor = vec4(color, 1.0);
        }
        else {
			FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		}
        return;
	}

    // Traverse the linked list
    while (currentEventPointer != 0) {
        RayEvent event = rayEvents[currentEventPointer];

        // Visualisation of error events
        if (pixelIndex >= heads.length()) {
            FragColor = vec4(1, 1, 0, 1); // Yellow for head OOB
            return;
        }
        if (currentEventPointer >= rayEvents.length()) {
            FragColor = vec4(vec3(currentEventPointer / float(rayEvents.length() * 3)), 1); // White for node OOB
            return;
        }

        float startT = event.distanceFromCamera;
        float endT = event.distanceFromCamera;
        bool hasNextEvent = (event.next != 0);
        
        // ======= TODO: IMPLEMENT ========
        // Decide what to do with the current node
        // Take into account whether the ray is entering or exiting a node
        // and whether the node is empty or filled
        
         bool shouldSample = false;

        // Entering a node
        if (event.isEntry == 1) {
            // Handle ray entry event

            if (event.occupancyClass == EVENT_OCCUPANCY_NONEMPTY && hasNextEvent) {
                RayEvent nextEvent = rayEvents[event.next];

                startT = event.distanceFromCamera;
                endT = nextEvent.distanceFromCamera;

                if (endT < startT) {
                    float temp = startT;
                    startT = endT;
                    endT = temp;
                }
                shouldSample = true;

            }

        }
        // Exiting a node
        else {
            // Handle ray exit event

        }

        if (!shouldSample || endT <= startT) {
            currentEventPointer = event.next;
            continue;
        }

        // Perform the sampling operation
        vec3 color;
        bool hit = sampleRange(startT, endT, direction, cameraPos, color);

        if (hit) {
            FragColor = vec4(color, 1.0);
            return;
        }

        // Move to the next node
        currentEventPointer = event.next;
    }

	FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}