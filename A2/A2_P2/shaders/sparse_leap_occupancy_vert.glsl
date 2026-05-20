#version 410
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 texCoord;

uniform mat4 u_modelViewProjection;
uniform mat4 u_model;
uniform vec3 cubeSize;

void main() {
    gl_Position = vec4(pos, 1.0);
}
