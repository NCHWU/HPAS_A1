#version 410 core
layout(location = 0) in vec2 inPos;
layout(location = 1) in int inLabel;

uniform mat4 view;
uniform mat4 projection;
uniform vec2 offset;
uniform vec2 scalingFactor;
uniform vec2 min;
uniform vec2 max;
uniform bool useLabel;

out vec3 labelColor;
out float show;

// List of 20 distinct colors
vec3 colors[20] = vec3[](
    vec3(1.0, 0.0, 0.0),  // Red
    vec3(0.0, 1.0, 0.0),  // Green
    vec3(0.0, 0.0, 1.0),  // Blue
    vec3(1.0, 0.7, 0.2),  // Yellowish
    vec3(1.0, 0.0, 1.0),  // Magenta
    vec3(0.0, 1.0, 1.0),  // Cyan
    vec3(0.5, 0.0, 0.5),  // Purple
    vec3(1.0, 0.5, 0.0),  // Orange
    vec3(0.5, 0.5, 0.0),  // Olive
    vec3(0.5, 0.0, 0.0),  // Maroon
    vec3(0.0, 0.5, 0.5),  // Teal
    vec3(0.0, 0.5, 1.0),  // Sky Blue
    vec3(0.5, 0.5, 0.5),  // Gray
    vec3(0.8, 0.0, 0.5),  // Rose
    vec3(0.2, 0.7, 0.3),  // Lime Green
    vec3(0.9, 0.3, 0.2),  // Coral
    vec3(0.3, 0.2, 0.7),  // Indigo
    vec3(0.7, 0.8, 0.2),  // Yellow-Green
    vec3(0.3, 0.3, 0.8),  // Periwinkle
    vec3(0.8, 0.6, 0.2)   // Gold
);

vec3 getRandomLabelColor(int label) {
    return colors[label % 20];
}

void main()
{
    if (inPos.x < min.x || inPos.x > max.x || inPos.y < min.y || inPos.y > max.y) {
        show = 0.;
    }
    else {
        show = 1.;
    }

    if (useLabel) {
        labelColor = getRandomLabelColor(inLabel);
    }
    else {
        labelColor = vec3(1., 0., 0.);
    }

    // Calculating screen position
    gl_Position = projection * view * vec4((inPos + offset) * scalingFactor, 0.0, 1.0);
}

