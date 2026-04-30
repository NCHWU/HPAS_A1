#version 410 core

uniform vec2 min;
uniform vec2 max;
in vec3 color;
in vec2 worldPosition;
out vec4 FragColor;

void main()
{
    if (worldPosition.x < min.x || worldPosition.x > max.x || worldPosition.y < min.y || worldPosition.y > max.y) {
        discard;
    }

    FragColor = vec4(color, 1.);
}