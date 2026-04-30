#version 410 core

in vec3 color;
in float show;
out vec4 FragColor;

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

    FragColor = vec4(color, 1.);
}