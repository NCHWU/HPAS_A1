#version 410 core
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 offset;
uniform vec3 scalingFactor;
uniform vec2 min;
uniform vec2 max;

out vec3 color;
out float show;

void main()
{
    if (inPos.x < min.x || inPos.x > max.x || inPos.y < min.y || inPos.y > max.y) {
        show = 0.;
    }
    else {
		show = 1.;
    }

	color = inColor;
	// Calculating screen position
    gl_Position = projection * view * vec4((inPos + offset) * scalingFactor, 1.0);
}