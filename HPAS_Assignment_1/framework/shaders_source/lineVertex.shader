#version 410 core
layout(location = 0) in vec3 inPos;

uniform mat4 view;
uniform mat4 projection;

void main()
{
	// Calculating screen position
	gl_Position = projection * view * vec4(inPos, 1.0);
}