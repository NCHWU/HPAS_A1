#version 410 core
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 offset;
uniform vec3 scalingFactor;
out vec3 color;
out vec2 worldPosition;

void main()
{
	worldPosition = inPos.xy;
	color = inColor;
	// Calculating screen position
    gl_Position = projection * view * vec4((inPos + offset) * scalingFactor, 1.0);
}