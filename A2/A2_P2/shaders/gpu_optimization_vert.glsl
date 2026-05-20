#version 410
layout(location = 0) in vec3 pos;

uniform mat4 u_modelViewProjection;
uniform mat4 u_model;

out vec3 u_color;
out vec3 worldPos;

void main() {
    // moves the cube instance to the correct position and clamps the last row of cubes
    vec3 actualPos = clamp(pos, 0, 1);
        
    // calculate position in view space
    worldPos = (u_model * vec4(actualPos, 1.0)).xyz;
    gl_Position = u_modelViewProjection * vec4(actualPos, 1.0);
        
    // assign a color the corner
    // this would be an alternative place to fix the ratio mismatch
    // by adjusting the color before it goes to the fragment shaders
    u_color = actualPos;
}
