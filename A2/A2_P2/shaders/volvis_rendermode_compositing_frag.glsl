#version 410
#extension GL_ARB_shader_storage_buffer_object : enable
#extension GL_ARB_shader_atomic_counters : enable

out vec4 FragColor;

uniform int SCREEN_WIDTH;
uniform vec2 screenSize;
uniform float fov;
uniform mat4 viewMatrix;
uniform vec3 cameraPos;
uniform float stepSize;
uniform bool doCountSamples;

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

in vec2 TexCoords;

// the front faces texture
uniform sampler2D frontFaces;

// the back faces texture
uniform sampler2D backFaces;

// the volume or volume cache when using indirection
uniform sampler3D volumeData;

// the volume indirection lookup
uniform sampler3D volumeIndexData;

// the transferfunction (2D for simplicity, values in y do not change, so it can be sampled with (norm intensity, 0.5)
uniform sampler2D transferFunction;

// this contains the voxels size in normalized coordinates + 0 if using regular texture and 1 when using bricking
uniform vec4 volumeInfo; // (voxelsize.x, voxelsize.y, voxelsize.z, use bricking?)
uniform vec2 volumeMaxValues; // 1/max intensity, 1/max gm

// contains various rendering options, here stepsize and its reciprocal (both in normalized volume space) and the stepsize in the original space, and the toggle to use shading
// Note: we give reciprocals to avoid divisions as they are more expensive than multiplications
// so if we can calculate a reciprocal once outside instead of potentially multiple times every thread it can save a lot of computation
uniform vec4 renderOptions; // (stepSize (adjusted to 0..1 volume coords), 1.0f / stepSize, stepSize (orginal value relative to default), use shading)

// Phong shading constants, defined globally to avoid repeated creation in phongShading function
const float ambientCoefficient = 0.1;
const float diffuseCoefficient = 0.6;
const float specularCoefficient = 0.5;
const int specularPower = 32;

uniform int doEmptySpaceSkipping;

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
void sampleRange(float startT, float endT, vec3 direction, vec3 cameraPos, inout vec3 C, inout float A) {
    for(float t = startT; t <= endT; t += renderOptions.z) {
        if (doCountSamples) {
            atomicCounterIncrement(sampleCounter);
        }

        // ====== TODO: IMPLEMENT ========
        // Sample the volume data and use it to composite the samples in C and A

        vec3 samplePos = cameraPos + t * direction;

        float intensity = sampleData(samplePos);
        float normalizedIntensity = clamp(intensity * volumeMaxValues.x, 0.0, 1.0);

        vec4 sampleColor = texture(transferFunction, vec2(normalizedIntensity, 0.5));

        float alpha = sampleColor.a;

        C += (1.0 - A) * alpha * sampleColor.rgb;
        A += (1.0 - A) * alpha;

        if (A > 0.99) {
            break;
        }


    }
}

void main()
{
    uint pixelIndex = uint(gl_FragCoord.y) * SCREEN_WIDTH + uint(gl_FragCoord.x);
    uint currentEventPointer = heads[pixelIndex];
    
    vec3 direction = getRayDirection(gl_FragCoord.xy);

    vec3 C = vec3(0.0);
    float A = 0.0;
    
    // If empty space skipping is disabled, simply use the start and end positions
    // to sample all along the volume data (naive approach)
    if (doEmptySpaceSkipping == 0) {
        vec3 startPos = texture(frontFaces, TexCoords).xyz / volumeInfo.xyz;
        vec3 endPos = texture(backFaces, TexCoords).xyz/ volumeInfo.xyz;

        float startT = length(startPos - cameraPos);
        float endT = length(endPos - cameraPos);

        sampleRange(startT, endT, direction, cameraPos, C, A);
        
        FragColor = vec4(C, A);
        //FragColor = vec4(endPos* volumeInfo.xyz, A);
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
        sampleRange(startT, endT, direction, cameraPos, C, A);

        // Move to the next node
        currentEventPointer = event.next;
    }

    // this sets the final color to the pixel
    FragColor = vec4(C, A);
}