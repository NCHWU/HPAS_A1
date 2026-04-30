#version 410 core

out vec4 FragColor;
in vec3 labelColor;
in float show;

uniform vec3 color;

void main()
{
    if (show == 0.) {
        discard;
    }

    // Render circles
    vec2 circCoord = 2.0 * gl_PointCoord - 1.0;
    if (dot(circCoord, circCoord) > 1.0) {
        discard;
    }

    FragColor = vec4(labelColor, 1.);
}